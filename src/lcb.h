#ifndef LCBPROXY_LCB_H
#define LCBPROXY_LCB_H
#include "common.h"

namespace Epoxy {
class LCBHandle;
class Command;

class LCBHandle {
public:
    LCBHandle(const std::string& host, const std::string& bucket, struct ev_loop *loop);
    ~LCBHandle() {}
    /**
     * Dispatch a command and await a response
     * @param cmd The command to dispatch
     * @return true on success, false on error
     */
    bool dispatch(Command *cmd);
    void flushQueue();

private:
    lcb_t instance;
    lcb_io_opt_t io;
    /** Queued command objects for when the instance is not yet bootstrapped */
    std::vector<Command *> cmdQueue;

    /** Reference count */
    size_t refcount;
};

} //namespace
#endif
