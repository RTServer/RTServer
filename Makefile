
default:	build

clean:
	rm -rf objs/bin/* \
		objs/src/core/* \
		objs/src/client/* \
		objs/src/db/* \
		objs/src/md5/* \
		objs/src/tools/* \
		objs/src/json/* \
		objs/src/hashmap/*

build:
	$(MAKE) -f objs/Makefile.Server
	$(MAKE) -f objs/Makefile.Client
	$(MAKE) -f objs/Makefile.Admin
	$(MAKE) -f objs/Makefile.Test

install:
	$(MAKE) -f objs/Makefile install

