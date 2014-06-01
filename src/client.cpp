#include "common.h"
#include "client.h"
#include "lcb.h"
#include "command.h"
#include "daemon.h"
#include "bufpair.h"

using namespace Epoxy;
using std::string;
using std::list;
typedef list<BufPair>::iterator BLIter;
typedef list<Buffer*>::iterator BPIter;

EnteredContext::EnteredContext(Client *c)
{
    client = c;
    client->entered = true;
    client->ref();
}

EnteredContext::~EnteredContext()
{
    client->entered = false;
    client->scheduleNocheck();
    client->unref();
}

extern "C" {

static void
watcher_cb(struct ev_loop *l, ev_io *io, int events)
{
    Client *client = (Client*)io->data;
    EnteredContext ctx(client);

    if (events & EV_READ) {
        client->slurp();
    }
    if (events & EV_WRITE) {
        client->drain();
    }
}
}


Client::Client(Daemon *d, int fd)
{
    parent = d;
    sockfd = fd;
    refcount = 1;
    watchedFor = 0;
    entered = false;
    closed = false;
    rope = new Rope(&d->pool);
    ev_init(&watcher, watcher_cb);
}

void
Client::scheduleNocheck()
{
    if (closed) {
        return;
    }

    short wanted = EV_READ;
    if (sendQueue.size()) {
        wanted |= EV_WRITE;
    }

    if (watchedFor != wanted) {
        ev_io_stop(parent->loop, &watcher);
        ev_io_set(&watcher, sockfd, wanted);
        ev_io_start(parent->loop, &watcher);
        watchedFor = wanted;
    }
}

void
Client::drain()
{
    struct msghdr mh;
    memset(&mh, 0, sizeof mh);
    struct iovec iov[EPOXY_NWRITE_IOV];

    BLIter ii = sendQueue.begin();
    unsigned jj = 0;
    for (; ii != sendQueue.end() && jj < EPOXY_NWRITE_IOV; ++ii, ++jj) {
        ii->fill(&iov[jj]);
    }

    mh.msg_iov = iov;
    mh.msg_iovlen = jj;
    ssize_t nw = sendmsg(sockfd, &mh, 0);

    if (nw < 1) {
        checkError();
        schedule();
    }

    while (nw > 0 && sendQueue.empty() == false) {
        BufPair& bp = sendQueue.front();
        nw -= bp.consumed(nw);
        if (bp.empty()) {
            bp.release();
            sendQueue.pop_front();
        }
    }
    schedule();
}

void
Client::slurp()
{
    ReadContext rctx;
    rope->getReadBuffers(rctx);
    struct msghdr mh;
    memset(&mh, 0, sizeof mh);

    mh.msg_iov = rctx.iov;
    mh.msg_iovlen = EPOXY_NREAD_IOV;
    ssize_t nr = recvmsg(sockfd, &mh, 0);
    if (nr <= 0) {
        checkError();
    }

    rope->setReadAdded(rctx, nr);

    while (true) {
        RequestHeader hdr;
        Command *cmd;

        bool rv = rope->getContig((char*)hdr.bytes, sizeof hdr.bytes);
        if (!rv) {
            break;
        }

        size_t pktsize = getPacketSize(hdr);
        if (pktsize > rope->size()) {
            break;
        }

        if (pktsize <= SmallCommand::maxPacketSize) {
            cmd = new SmallCommand(hdr, rope);
        } else {
            cmd = new PayloadCommand(hdr, rope);
        }
        parent->getHandle()->dispatch(cmd);
    }

    schedule();
}

void
Client::gotResponse(Command *cmd, lcb_PKTFWDRESP* resp)
{
    ResponseHeader hdr;
    memcpy(hdr.bytes, resp->header, sizeof hdr.bytes);

    lcb_IOV *first = &resp->iovs[0];
    assert(first->iov_len >=  sizeof hdr.bytes);
    hdr.response.opaque = cmd->getOpaque();
    memcpy(first->iov_base, hdr.bytes, sizeof hdr.bytes);

    for (unsigned ii = 0; ii < resp->nitems; ++ii) {
        sendQueue.push_back(BufPair(resp->iovs[ii], resp->bufs[ii]));
        lcb_backbuf_ref(resp->bufs[ii]);
    }
    schedule();
}

void
Client::gotResponse(Command *cmd, lcb_error_t err)
{
    ResponseHeader hdr;
    memset(&hdr, 0, sizeof hdr);
    hdr.response.magic = PROTOCOL_BINARY_RES;
    hdr.response.opcode = cmd->getOpcode();
    hdr.response.opaque = cmd->getOpaque();
    hdr.response.status = htons(PROTOCOL_BINARY_RESPONSE_ETMPFAIL);
    sendQueue.push_back(BufPair((char*)hdr.bytes, sizeof hdr.bytes));
    schedule();
}
