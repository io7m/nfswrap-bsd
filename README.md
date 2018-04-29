nfswrap-bsd
===

A wrapper for the [BSD nfsd](https://man.openbsd.org/nfsd) that allows for
[process supervision](http://smarden.org/runit/).

Requirements
===

* FreeBSD 11+
* OpenBSD 6.3+ (Untested)

Compiling
===

```
make && sudo cp nfsd-wrap /usr/local/bin/nfsd-wrap
```

Usage
===

```
nfsd-wrap: usage: nfsd.sh
```

`nfsd.sh` must be a program that starts `nfsd` and waits for it to exit. An example of
a correct `nfsd.sh` program might be:

```
#!/bin/sh
exec /usr/sbin/nfsd --debug -h 127.0.0.1 -t -n 4
```

The `--debug` flag is an undocumented (and dangerous) flag to the BSD
`nfsd`. When `nfsd` is run with the `--debug` flag, it will not fork
into the background. Unfortunately, it will also do something rather
dangerous as will now be explained.

In normal operation `nfsd` will declare some signal handlers that
ignore signals such as `SIGTERM`, `SIGINT`, `SIGHUP`. During operation,
`nfsd` allocates some kernel resources that are _only_ cleaned up
if the process exits by receiving a `SIGUSR1` signal. If the kernel
resources that `nfsd` allocates are not cleaned up, the `nfsd` process
will simply crash the next time it is run. There appears to be no way
to fix this beyond rebooting the entire system. Unfortunately, when the
`--debug` flag is used, the `nfsd` process does not set up any signal
handlers that ignore signals other than `SIGUSR1`, meaning that it is
very easy to shut down the `nfsd` process incorrectly and effectively
prevent the process from running again until the next reboot.

The `nfsd-wrap` program acts as a wall between the real `nfsd` process
and the process supervision system (such as `runit`). If the `nfsd-wrap`
process is sent `SIGINT`, `SIGHUP`, `SIGTERM`, or `SIGUSR1`, it sends
`SIGUSR1` to the `nfsd` process. This ensures that the correct cleanup
occurs.

