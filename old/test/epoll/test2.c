//http://www.cnblogs.com/haippy/archive/2012/01/09/2317269.html
//gcc -o test2 test2.c -I /opt/local/include

#include <stdio.h>
#include <event2/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//create_and_bind() 包含了如何创建 IPv4 和 IPv6 套接字的代码块，它接受一字符串作为端口参数，并在 result 中返回一个 addrinfo 结构，如果函数成功则返回套接字，如果失败，则返回 -1，
static int
create_and_bind (char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;
      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }
  if (rp == NULL)
    {
      fprintf (stderr, "Could not bind\n");
      return -1;
    }
  freeaddrinfo (result);
  return sfd;
}

struct addrinfo
{
  int              ai_flags;
  int              ai_family;
  int              ai_socktype;
  int              ai_protocol;
  size_t           ai_addrlen;
  struct sockaddr *ai_addr;
  char            *ai_canonname;
  struct addrinfo *ai_next;
};


//下面，我们将一个套接字设置为非阻塞形式，函数如下：
static int
make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

//接下来，便是主函数代码，主要用于事件循环：
#define MAXEVENTS 64

int
main (int argc, char *argv[])
{
  int sfd, s;
  int efd;
  struct epoll_event event;
  struct epoll_event *events;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s [port]\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  sfd = create_and_bind (argv[1]);
  if (sfd == -1)
    abort ();

  s = make_socket_non_blocking (sfd);
  if (s == -1)
    abort ();

  s = listen (sfd, SOMAXCONN);
  if (s == -1)
    {
      perror ("listen");
      abort ();
    }

  efd = epoll_create1 (0);
  if (efd == -1)
    {
      perror ("epoll_create");
      abort ();
    }

  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
  if (s == -1)
    {
      perror ("epoll_ctl");
      abort ();
    }

  /* Buffer where events are returned */
  events = calloc (MAXEVENTS, sizeof event);

  /* The event loop */
  while (1)
    {
      int n, i;

      n = epoll_wait (efd, events, MAXEVENTS, -1);
      for (i = 0; i < n; i++)
    {
      if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
        {
              /* An error has occured on this fd, or the socket is not
                 ready for reading (why were we notified then?) */
          fprintf (stderr, "epoll error\n");
          close (events[i].data.fd);
          continue;
        }

      else if (sfd == events[i].data.fd)
        {
              /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
              while (1)
                {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  in_len = sizeof in_addr;
                  infd = accept (sfd, &in_addr, &in_len);
                  if (infd == -1)
                    {
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                        {
                          /* We have processed all incoming
                             connections. */
                          break;
                        }
                      else
                        {
                          perror ("accept");
                          break;
                        }
                    }

                  s = getnameinfo (&in_addr, in_len,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
                  if (s == 0)
                    {
                      printf("Accepted connection on descriptor %d "
                             "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                  /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                  s = make_socket_non_blocking (infd);
                  if (s == -1)
                    abort ();

                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                    {
                      perror ("epoll_ctl");
                      abort ();
                    }
                }
              continue;
            }
          else
            {
              /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
              int done = 0;

              while (1)
                {
                  ssize_t count;
                  char buf[512];

                  count = read (events[i].data.fd, buf, sizeof buf);
                  if (count == -1)
                    {
                      /* If errno == EAGAIN, that means we have read all
                         data. So go back to the main loop. */
                      if (errno != EAGAIN)
                        {
                          perror ("read");
                          done = 1;
                        }
                      break;
                    }
                  else if (count == 0)
                    {
                      /* End of file. The remote has closed the
                         connection. */
                      done = 1;
                      break;
                    }

                  /* Write the buffer to standard output */
                  s = write (1, buf, count);
                  if (s == -1)
                    {
                      perror ("write");
                      abort ();
                    }
                }

              if (done)
                {
                  printf ("Closed connection on descriptor %d\n",
                          events[i].data.fd);

                  /* Closing the descriptor will make epoll remove it
                     from the set of descriptors which are monitored. */
                  close (events[i].data.fd);
                }
            }
        }
    }

  free (events);

  close (sfd);

  return EXIT_SUCCESS;
}

/**
main() 首先调用 create_and_bind() 建立套接字，然后将其设置为非阻塞的，再调用 listen(2)。之后创建一个epoll 实例 efd（文件描述符），并将其加入到sfd的监听套接字中以边沿触发方式等待事件输入。

外层的 while 循环是主事件循环，它调用了 epoll_wait(2)，此时线程仍然被阻塞等待事件，当事件可用时，epoll_wait(2) 将会在events参数中返回可用事件。

epoll 实例 efd 在每次事件到来并需要添加新的监听时就会得到更新，并删除死亡的链接。

当事件可用时，可能有一下三种类型：

    Errors: 当错误情况出现时，或者不是与读取数据相关的事件通告，我们只是关闭相关的描述符，关闭该描述符会自动的将其从被epoll 实例 efd 监听的的集合中删除。
    New connections: 当监听的文件描述符 sfd 可读时，此时会有一个或多个新的连接到来，当新连接到来时，accept(2) 该连接，并打印一条信息，将其设置为非阻塞的并把它加入到被 epoll 实例监听的集合中。
    Client data: 当数据在客户端描述符可用时，我们使用 read(2) 在一个内部循环中每次读取512 字节数据。由于我们必须读取所有的可用数据，此时我们并不能获取更多的事件，因为描述符是以边沿触发监听的，读取的数据被写到 stdout (fd=1) (write(2))。如果 read(2) 返回 0，意味着到了文件末尾EOF，我们可以关闭客户端连接，如果返回  -1， errno 会被设置成 EAGAIN, 这意味着所有的数据已经被读取，可以返回主循环了。
*/

