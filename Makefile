CC=      clang
CC_ARGS= -g -W -Wall -pedantic --std=c99

all: nfsd-wrap nfsd

nfsd-wrap: nfsd-wrap.c
	$(CC) $(CC_ARGS) -o nfsd-wrap nfsd-wrap.c

nfsd: nfsd.c
	$(CC) $(CC_ARGS) -o nfsd nfsd.c

clean:
	rm -f nfsd-wrap nfsd

