CC=      clang++
CC_ARGS= -g -W -Wall -pedantic --std=c++11

all: nfswrap rpcbind nfsd

nfswrap: nfswrap.cpp
	$(CC) $(CC_ARGS) -o nfswrap nfswrap.cpp -lkvm

rpcbind: rpcbind.cpp
	$(CC) $(CC_ARGS) -o rpcbind rpcbind.cpp

nfsd: nfsd.cpp
	$(CC) $(CC_ARGS) -o nfsd nfsd.cpp

clean:
	rm -f nfswrap rpcbind nfsd

