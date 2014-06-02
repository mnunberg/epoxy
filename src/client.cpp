#include "common.h"
#include "client.h"
#include "lcb.h"
#include "command.h"
#include "daemon.h"
#include "bufpair.h"
#include <errno.h>

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
watcher_cb(struct ev_loop *, ev_io *io, int events)
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
    ev_io_init(&watcher, watcher_cb, sockfd, EV_READ);
    watcher.data = this;
    schedule();
}

Client::~Client()
{
    closeSock();
    delete rope;
}

void
Client::scheduleNocheck()
{
    if (closed) {
        return;
    }

    short wanted = EV_READ;
    if (!sendQueue.empty()) {
        wanted |= EV_WRITE;
    }

    if (watchedFor != wanted) {
        ev_io_stop(parent->getLoop(), &watcher);
        ev_io_set(&watcher, sockfd, wanted);
        ev_io_start(parent->getLoop(), &watcher);
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
        checkError(nw, errno);
        schedule();
    } else {
        log_client_debug("Flushed %ld bytes", nw);
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
Client::sendConfigBlob(const RequestHeader& hdr)
{
    ResponseHeader res;
    memset(&res, 0, sizeof res);

    makeResponseTemplate(res, hdr);
    res.response.status = PROTOCOL_BINARY_RESPONSE_SUCCESS;

    // Get the config string:
    const string& blob = parent->getConfigBlob();
    BufPair bpBody(blob.c_str(), blob.size(), BufPair::STATIC);
    res.response.bodylen = htonl((uint32_t)blob.size());

    sendQueue.push_back(BufPair((const char*)res.bytes, sizeof res.bytes));
    sendQueue.push_back(bpBody);
    schedule();
}

static const char* Sasl_Mech_Response = "PLAIN";
static const int Sasl_Mech_Length = sizeof("PLAIN")-1;

void
Client::sendSaslMechs(const RequestHeader& hdr)
{
    ResponseHeader res;
    memset(&res, 0, sizeof res);
    makeResponseTemplate(res, hdr);
    res.response.status = PROTOCOL_BINARY_RESPONSE_SUCCESS;

    const char *body = Sasl_Mech_Response;
    const size_t nbody = Sasl_Mech_Length;

    res.response.bodylen = ntohl((uint32_t)nbody);
    sendQueue.push_back(BufPair((const char*)res.bytes, sizeof res.bytes));
    sendQueue.push_back(BufPair(body, nbody, BufPair::STATIC));
    schedule();
}

void
Client::sendError(const RequestHeader& hdr, uint16_t rc)
{
    ResponseHeader res;
    memset(&res, 0, sizeof res);
    makeResponseTemplate(res, hdr);
    res.response.status = htons(rc);
    sendQueue.push_back(BufPair((const char*)res.bytes, sizeof res.bytes));
    schedule();
}

bool
Client::readCommand()
{
    RequestHeader hdr;
    Command *cmd;

    bool rv = rope->getContig((char*)hdr.bytes, sizeof hdr.bytes);
    if (!rv) {
        return false;
    }

    size_t pktsize = getPacketSize(hdr);
    if (pktsize > rope->size()) {
        // Have packet, but not enough body
        return false;
    }

    log_client_trace("Read packet of size %lu", pktsize);

    switch (hdr.request.opcode) {
    case PROTOCOL_BINARY_CMD_GET:
    case PROTOCOL_BINARY_CMD_GAT:
    case PROTOCOL_BINARY_CMD_ADD:
    case PROTOCOL_BINARY_CMD_SET:
    case PROTOCOL_BINARY_CMD_REPLACE:
    case PROTOCOL_BINARY_CMD_INCREMENT:
    case PROTOCOL_BINARY_CMD_DECREMENT:
    case PROTOCOL_BINARY_CMD_GET_LOCKED:
    case PROTOCOL_BINARY_CMD_UNLOCK_KEY:
    case PROTOCOL_BINARY_CMD_DELETE:
    case PROTOCOL_BINARY_CMD_APPEND:
    case PROTOCOL_BINARY_CMD_PREPEND: {
        if (pktsize <= EPOXY_SMALLCMD_MAXSZ) {
            cmd = new SmallCommand(hdr, rope);
        } else {
            cmd = new PayloadCommand(hdr, rope);
        }

        LCBHandle *handle = parent->getHandle();
        handle->dispatch(cmd);
        cmd->setDispatched(this, handle);
        ref();
        schedule();
        return true;
    }

    case PROTOCOL_BINARY_CMD_GET_CLUSTER_CONFIG:
        sendConfigBlob(hdr);
        break;

    case PROTOCOL_BINARY_CMD_SASL_LIST_MECHS:
        sendSaslMechs(hdr);
        break;

    case PROTOCOL_BINARY_CMD_SASL_AUTH:
        sendError(hdr, PROTOCOL_BINARY_RESPONSE_SUCCESS);
        break;

    case PROTOCOL_BINARY_CMD_GET_REPLICA:
    case PROTOCOL_BINARY_CMD_OBSERVE:
    case PROTOCOL_BINARY_CMD_SASL_STEP:
    case PROTOCOL_BINARY_CMD_VERBOSITY:
        sendError(hdr, PROTOCOL_BINARY_RESPONSE_NOT_SUPPORTED);
        break;

    case PROTOCOL_BINARY_CMD_QUIT:
        closeSock();
        return false;

    default:
        sendError(hdr, PROTOCOL_BINARY_RESPONSE_UNKNOWN_COMMAND);
        break;
    }
    rope->consumed(pktsize);
    ref();
    schedule();
    return true;
}

void
Client::slurp()
{
    ReadContext rctx;
    struct msghdr mh;

    memset(&mh, 0, sizeof mh);
    rope->getReadBuffers(rctx);
    mh.msg_iov = rctx.iov;
    mh.msg_iovlen = EPOXY_NREAD_IOV;
    ssize_t nr = recvmsg(sockfd, &mh, 0);

    if (nr > 0) {
        log_client_trace("Read %ld bytes", nr);
        rope->setReadAdded(rctx, nr);
        while (readCommand());
    } else {
        checkError(nr, errno);
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
    unref();
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
    unref();
}

void
Client::closeSock()
{
    if (sockfd < 0) {
        return;
    }

    if (watchedFor) {
        ev_io_stop(parent->getLoop(), &watcher);
    }
    close(sockfd);
    sockfd = -1;
    closed = true;
}

void
Client::checkError(int rv, int errcurr)
{
    if (rv == 0) {
        log_client_warn("Client %d closed the connection", sockfd);
        closeSock();
    } else if (rv == -1) {
        switch (errcurr) {
        case EWOULDBLOCK:
        case EINTR:
            return;
        default:
            log_client_error("Received error on socket %d: %s",
                sockfd, strerror(errcurr));
            closeSock();
        }
    }
}
