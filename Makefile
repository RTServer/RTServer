
default:	build

clean:
	rm -rf objs/RTServer \
		objs/src/core/* \
		objs/src/client/* \
		objs/src/db/* \
		objs/src/md5/* \
		objs/src/tools/* \
		objs/src/json/*

build:
	$(MAKE) -f objs/Makefile

install:
	$(MAKE) -f objs/Makefile install

