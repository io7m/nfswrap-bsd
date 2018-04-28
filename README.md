nfswrap-bsd
===

A wrapper for the [https://man.openbsd.org/nfsd](BSD nfsd) that allows for
[process supervision](http://smarden.org/runit/).

Requirements
===

* FreeBSD 11+
* OpenBSD 6.3+ (Untested)

Compiling
===

```
make && sudo cp nfswrap /usr/local/bin/nfswrap
```

Usage
===

```
nfswrap: usage: rpcbind.sh nfsd.sh
```

`rpcbind.sh` must be a program that starts `rpcbind` and exits when `rpcbind` exits. An
example of a correct `rpcbind.sh` program might be:

```
#!/bin/sh
exec /usr/sbin/rpcbind -d -h 127.0.0.1 -s
```

`nfsd.sh` must be a program that starts `nfsd` and exits immediately. An example of
a correct `nfsd.sh` program might be:

```
#!/bin/sh
exec /usr/sbin/nfsd -h 127.0.0.1 -t -n 4
```

This works because the BSD `nfsd` unconditionally forks into the background and cannot
be forced into the foreground.

The `nfswrap` program first starts `rpcbind.sh`, watches `rpcbind.sh` for five seconds,
then starts `nfsd.sh`. If `rpcbind` crashes or fails to start, or if `nfsd` crashes or
fails to start, `nfswrap` attempts to clean up all processes and then exits. If `nfswrap`
receives `SIGTERM`, `SIGINT`, or `SIGHUP`, then it cleans up all `rpcbind` and `nfsd`
processes and then exits.

