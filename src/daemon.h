#ifndef LCBPROXY_DAEMON_H
#define LCBPROXY_DAEMON_H
#include "common.h"
#include "buffer.h"
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace Epoxy {
class LCBHandle;

class Daemon {
public:
    Daemon(const std::string& bname, int lsnport);
    void run();
    void acceptClient();
    const std::string& getConfigBlob() const { return jsonConfig; }

    BufferPool pool;
    std::list<LCBHandle *> handles;
    LCBHandle * getHandle();
    struct ev_loop * getLoop() const { return loop; }

private:
    void reschedule();
    void genConfig();
    int lsnfd;
    ev_io watcher;
    struct sockaddr_in lsnaddr;
    const std::string bucket;
    struct ev_loop *loop;
    std::string jsonConfig;
};

}//namespace
#endif
