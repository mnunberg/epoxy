#include "daemon.h"
#include "lcb.h"
#include "client.h"
#include <fcntl.h>
#include <errno.h>
#include <cassert>

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
    Client *c = new Client(this, newfd);
    reschedule();
}

Daemon::Daemon(const string& bname, int lsnport) :bucket(bname)
{
    loop = ev_default_loop(0);
    memset(&lsnaddr, 0, sizeof lsnaddr);
    lsnaddr.sin_addr.s_addr = INADDR_ANY;
    lsnaddr.sin_family = AF_INET;
    lsnaddr.sin_port = htons((uint16_t)lsnport);
    lsnfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (lsnfd == -1) {
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

    // Create our instance
    LCBHandle *handle = new LCBHandle("localhost", bucket, loop);
    handles.push_back(handle);
}

void
Daemon::run()
{
    ev_loop(loop, 0);
}

LCBHandle *
Daemon::getHandle()
{
    return handles.front();
}
