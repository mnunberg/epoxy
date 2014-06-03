# Epoxy - Couchbase Proxy

This is a sample application used to demonstrate and test the packet forwarding
functionalities of libcouchbase. Currently this depends on my fork (using the
`packet-ng` branch) at https://github.com/mnunberg/libcouchbase.

# Building

```
$ git submodule init
$ git submodule update
$ mkdir build
$ cd build
$ cmake -DLCB_ROOT=/path/to/lcb/installation ../
$ make
$ ./epoxy
```

By default `epoxy` will listen on port `4444`. It will act as a simple memcached
node.

It will respond with a customized configuration for CCCP config requests and will
blindly accept any form of SASL auth. Simple old-style memached single-packet
commands are recognized and forwarded through an internal client.

# TODO

* Handle stats, observe
* Allow to configure how many instances are spawned
* Allow to provide more options about the target server
