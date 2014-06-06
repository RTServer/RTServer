/* Port to listen on. */
#define SERVER_PORT 5566
/* Connection backlog (# of backlogged connections to accept). */
#define CONNECTION_BACKLOG 60000 /* 并发时监听队列长度 */
/* Socket read and write timeouts, in seconds. */
#define SOCKET_READ_TIMEOUT_SECONDS 3600
#define SOCKET_WRITE_TIMEOUT_SECONDS 3600
/* Number of worker threads.  Should match number of CPU cores reported in /proc/cpuinfo. */
#define NUM_THREADS 8
