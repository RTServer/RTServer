#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "transport.h"
#include "../json/cJSON.h"

_RTS_TRANSPORT_DATA RTS_transport_data_init() {
	static _RTS_TRANSPORT_DATA _rts_transport_data; 
	_rts_transport_data.id = 0;
	_rts_transport_data.toid = 0;
    memset(_rts_transport_data.action, 0, MAX_ACTION_LENGTH + 1);
    memset(_rts_transport_data.name, 0, MAX_NAME_LENGTH + 1);
    memset(_rts_transport_data.token, 0, MAX_TOKEN_LENGTH + 1);
    memset(_rts_transport_data.password, 0, MAX_PASSWORD_LENGTH + 1);
    wmemset(_rts_transport_data.content, 0, MAX_CONTENT_LENGTH + 1);
    return _rts_transport_data;
}

int RTS_transport_data_parse(const char *buf, _RTS_TRANSPORT_DATA *_rts_transport_data) {
	cJSON *json = NULL, *json_tmp = NULL;
    json = cJSON_Parse(buf);
    if (!json) {
    	return 0;
    }
    (json_tmp = cJSON_GetObjectItem(json, "action")) && strncpy(_rts_transport_data->action, json_tmp->valuestring, MAX_ACTION_LENGTH);
    (json_tmp = cJSON_GetObjectItem(json, "name")) && strncpy(_rts_transport_data->name, json_tmp->valuestring, MAX_NAME_LENGTH);
    (json_tmp = cJSON_GetObjectItem(json, "password")) && strncpy(_rts_transport_data->password, json_tmp->valuestring, MAX_PASSWORD_LENGTH);
    (json_tmp = cJSON_GetObjectItem(json, "token")) && strncpy(_rts_transport_data->token, json_tmp->valuestring, MAX_TOKEN_LENGTH);
    (json_tmp = cJSON_GetObjectItem(json, "toid")) && (_rts_transport_data->toid = json_tmp->valueint);
    (json_tmp = cJSON_GetObjectItem(json, "id")) && (_rts_transport_data->id = json_tmp->valueint);
    (json_tmp = cJSON_GetObjectItem(json, "content")) && mbstowcs(_rts_transport_data->content, json_tmp->valuestring, MAX_CONTENT_LENGTH);
    //释放内存
    cJSON_Delete(json);
    return 1;
}