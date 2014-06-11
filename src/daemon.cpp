#include "daemon.h"
#include "lcb.h"
#include "client.h"
#include <fcntl.h>
#include <errno.h>
#include <cassert>
#include <libcouchbase/vbucket.h>

using namespace Epoxy;
using std::string;

extern "C"
{
static void
accept_callback(struct ev_loop *, ev_io *io, int events)
{
    Daemon *d = (Daemon *)io->data;
    d->acceptClient();
}
}

void
Daemon::reschedule()
{
    if (ev_is_active(&watcher)) {
        ev_io_stop(loop, &watcher);
    }
    ev_io_start(loop, &watcher);
}

void
Daemon::acceptClient()
{
    struct sockaddr_in newaddr;
    socklen_t newlen = sizeof newaddr;
    int newfd = accept(lsnfd, (struct sockaddr*)&newaddr, &newlen);
    if (newfd == -1) {
        if (errno == EWOULDBLOCK || errno == EINTR) {
            reschedule();
            return;
        } else {
            abort();
        }
    }

    int rv = fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL)|O_NONBLOCK);
    if (rv != 0) {
        log_accept_crit("Couldn't set socket to nonblocking");
        abort();
    }

    new Client(this, newfd);
    reschedule();
}

Daemon::Daemon(const string& dsn, int lsnport) : dsn(dsn)
{
    loop = ev_default_loop(0);
    memset(&lsnaddr, 0, sizeof lsnaddr);
    lsnaddr.sin_addr.s_addr = INADDR_ANY;
    lsnaddr.sin_family = AF_INET;
    lsnaddr.sin_port = htons((uint16_t)lsnport);

    lsnfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (lsnfd < 0) {
        log_accept_crit("Couldn't create socket [%d] %s", errno, strerror(errno));
        abort();
    }

    int rv, optval;
    optval = 1;

    rv = setsockopt(lsnfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    assert(rv == 0);

    rv = bind(lsnfd, (struct sockaddr*)&lsnaddr, sizeof lsnaddr);
    assert(rv == 0);

    rv = fcntl(lsnfd, F_SETFL, fcntl(lsnfd, F_GETFL)|O_NONBLOCK);
    assert(rv == 0);

    rv = listen(lsnfd, 128);
    assert(rv == 0);

    ev_io_init(&watcher, accept_callback, lsnfd, EV_READ);
    ev_io_start(loop, &watcher);
    watcher.data = this;

    // Create our instance
    LCBHandle *handle = new LCBHandle(dsn, loop);
    handles.push_back(handle);
    genConfig();
}

void
Daemon::run()
{
    log_accept_info("Starting event loop");
    ev_loop(loop, 0);
}

LCBHandle *
Daemon::getHandle()
{
    return handles.front();
}

void
Daemon::genConfig()
{
    int rv;
    lcbvb_CONFIG *vbc = lcbvb_create();
    lcbvb_SERVER dummy;

    memset(&dummy, 0, sizeof dummy);
    dummy.hostname = (char *)"localhost";
    dummy.svc.data = ntohs(lsnaddr.sin_port);

    rv = lcbvb_genconfig_ex(vbc, "default", NULL, &dummy, 1, 0, 1);
    assert(rv == 0);
    char *configStr = lcbvb_save_json(vbc);
    jsonConfig = configStr;
    free(configStr);
    lcbvb_destroy(vbc);
}
