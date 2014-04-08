#include "const.h"

//定义传输数据结构
typedef struct _RTS_TRANSPORT_DATA {
	int id;
	int toid;
	char action[MAX_ACTION_LENGTH + 1];
	char name[MAX_NAME_LENGTH + 1];
	char password[MAX_PASSWORD_LENGTH + 1];
	char token[MAX_TOKEN_LENGTH + 1];
	wchar_t content[MAX_CONTENT_LENGTH + 1];
}_RTS_TRANSPORT_DATA;

extern int RTS_transport_data_parse(const char *buf, _RTS_TRANSPORT_DATA *_rts_transport_data);
extern _RTS_TRANSPORT_DATA RTS_transport_data_init();
extern int RTS_send(int sockfd, const char *content);