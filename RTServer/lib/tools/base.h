extern char *RTS_current_datetime();
extern char *RTS_unique();
extern void RTS_send(int sockfd, const char *content);
extern char *RTS_rand();
extern char *RTS_md5(char *str);
extern char *RTS_hash(char *password, char *salt);