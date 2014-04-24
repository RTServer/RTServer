#define RTS_DEBUG 1 //调试输出

extern char *RTS_current_datetime();
extern char *RTS_unique();
extern char *RTS_rand();
extern char *RTS_md5(char *str);
extern char *RTS_hash(char *password, char *salt);
extern int RTS_printf(const char *fmt, ...);