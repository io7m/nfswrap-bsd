CC=      clang++
CC_ARGS= -g -W -Wall -pedantic --std=c++11

all: nfsd-wrap rpcbind nfsd

nfsd-wrap: nfsd-wrap.cpp
	$(CC) $(CC_ARGS) -o nfsd-wrap nfsd-wrap.cpp -lkvm

rpcbind: rpcbind.cpp
	$(CC) $(CC_ARGS) -o rpcbind rpcbind.cpp

nfsd: nfsd.cpp
	$(CC) $(CC_ARGS) -o nfsd nfsd.cpp

clean:
	rm -f nfsd-wrap rpcbind nfsd

