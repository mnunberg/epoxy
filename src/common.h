#ifndef LCBPROXY_COMMON_H
#define LCBPROXY_COMMON_H
#include <libcouchbase/couchbase.h>
#include <libcouchbase/pktfwd.h>
#include <sys/uio.h>
#include <ev.h>
#include <cstdlib>
#include <list>
#include <vector>
#include <string>
extern "C" {
#include "logging/epoxy_yolog.h"
}

#define EPOXY_NREAD_IOV 16
#define EPOXY_NWRITE_IOV 32
#define EPOXY_SMALLCMD_MAXSZ 64

namespace Epoxy {
class Buffer;
class BufPair;
class BufferPool;
class Client;
class Command;
class Daemon;
class LCBHandle;
class Rope;

} //namespace

#endif
