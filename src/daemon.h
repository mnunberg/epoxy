#ifndef LCBPROXY_DAEMON_H
#define LCBPROXY_DAEMON_H
#include "common.h"
#include "buffer.h"
#include <ev.h>
#include <map>

namespace Epoxy {
class LCBHandle;

struct Daemon {
public:
    BufferPool pool;
    std::list<LCBHandle *> handles;
    struct ev_loop *loop;
    LCBHandle* getHandle();
};

}//namespace
#endif
