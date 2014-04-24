
CC =	cc
CFLAGS =  
CPP =	
LINK =	$(CC)


CORE_DEPS = src/core/RTServer.h \
	src/core/workqueue.h

CLIENT_DEPS = src/client/const.h \
	src/client/interface.h \
	src/client/transport.h

DB_DEPS = src/db/data.h

TOOLS_DEPS = src/tools/base.h

MD5_DEPS = src/md5/md5.h

JSON_DEPS = src/json/cJSON.h


CORE_INCS = -I /opt/local/include \
	-I objs

CLIENT_INCS = -I src/core \
	-I src/client

DB_INCS = -I src/core \
	-I src/db

TOOLS_INCS = -I src/core \
	-I src/tools

MD5_INCS = -I src/core \
	-I src/md5

JSON_INCS = -I src/core \
	-I src/json


objs/RTServer:	objs/src/core/RTServer.o \
	objs/src/core/workqueue.o \
	objs/src/client/interface.o \
	objs/src/client/transport.o \
	objs/src/db/data.o \
	objs/src/tools/base.o \
	objs/src/md5/md5.o \
	objs/src/json/cJSON.o

	$(LINK) -o objs/RTServer \
	objs/src/core/RTServer.o \
	objs/src/core/workqueue.o \
	objs/src/client/interface.o \
	objs/src/client/transport.o \
	objs/src/db/data.o \
	objs/src/tools/base.o \
	objs/src/md5/md5.o \
	objs/src/json/cJSON.o \
	-L/opt/local/lib -levent -lpthread -lm -lsqlite3



objs/src/core/RTServer.o:	$(CORE_DEPS) \
	src/core/RTServer.c

	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/RTServer.o \
		src/core/RTServer.c


objs/src/core/workqueue.o:	$(CORE_DEPS) \
	src/core/workqueue.c

	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/workqueue.o \
		src/core/workqueue.c


objs/src/client/interface.o:	$(CLIENT_DEPS) \
	src/client/interface.c

	$(CC) -c $(CFLAGS) $(CLIENT_INCS) \
		-o objs/src/client/interface.o \
		src/client/interface.c


objs/src/client/transport.o:	$(CLIENT_DEPS) \
	src/client/transport.c

	$(CC) -c $(CFLAGS) $(CLIENT_INCS) \
		-o objs/src/client/transport.o \
		src/client/transport.c


objs/src/db/data.o:	$(DB_DEPS) \
	src/db/data.c

	$(CC) -c $(CFLAGS) $(DB_INCS) \
		-o objs/src/db/data.o \
		src/db/data.c


objs/src/tools/base.o:	$(TOOLS_DEPS) \
	src/tools/base.c

	$(CC) -c $(CFLAGS) $(TOOLS_INCS) \
		-o objs/src/tools/base.o \
		src/tools/base.c


objs/src/md5/md5.o:	$(MD5_DEPS) \
	src/md5/md5.c

	$(CC) -c $(CFLAGS) $(MD5_INCS) \
		-o objs/src/md5/md5.o \
		src/md5/md5.c


objs/src/json/cJSON.o:	$(JSON_DEPS) \
	src/json/cJSON.c

	$(CC) -c $(CFLAGS) $(JSON_INCS) \
		-o objs/src/json/cJSON.o \
		src/json/cJSON.c

