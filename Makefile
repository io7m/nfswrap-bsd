all: nfswrap rpcbind nfsd

nfswrap: nfswrap.cpp
	clang++ -g -W -Wall -pedantic --std=c++11 -o nfswrap nfswrap.cpp -lkvm

rpcbind: rpcbind.cpp
	clang++ -g -W -Wall -pedantic --std=c++11 -o rpcbind rpcbind.cpp

nfsd: nfsd.cpp
	clang++ -g -W -Wall -pedantic --std=c++11 -o nfsd nfsd.cpp

clean:
	rm -f nfswrap rpcbind nfsd

