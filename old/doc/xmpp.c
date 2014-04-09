#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#define DEST_IP "113.105.65.227"
#define DEST_PORT 5222
#define MAX_BUF 1024
int main (int argc, char** argv){
  int sk ;
  sk =  socket(AF_INET,SOCK_STREAM,0);

  struct sockaddr_in dest_addr;//inet.h
  bzero(&dest_addr, sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(DEST_PORT);
  dest_addr.sin_addr.s_addr=inet_addr(DEST_IP);

  int cn;//result of excute "connect" function. 
  char * msg = (char *)malloc(MAX_BUF);//input string.
  int sendbytes;//the bytes of content. 
  char buf[MAX_BUF+1];//recive containter.
  char * breakflag = "done";//the flag of finished.
  //connect server
  cn = connect(sk,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
  printf("connect ...%d\n", cn);
  while(1){
    printf("\nINPUT:\n");
    scanf("%[^\n]%*c", msg);
    if(! strcmp(msg, breakflag))break;
    printf("the input msg is: %s\n", msg);
    //send msg
    sendbytes = send(sk, msg, strlen(msg), 0);
    bzero(buf, MAX_BUF+1);
    //recive remote response msg
    recv(sk, buf, MAX_BUF, 0);
    printf("recv: %s\n", buf);
  }
  close(sk);
  printf("Close ...");
}
