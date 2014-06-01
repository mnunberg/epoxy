#include "lcb.h"
#include "command.h"
#include "client.h"

using namespace Epoxy;
using std::string;
using std::vector;

extern "C"
{
static void
bootstrap_callback(lcb_t instance, lcb_error_t err)
{
    assert(err == LCB_SUCCESS);
    LCBHandle *handle = (LCBHandle *)lcb_get_cookie(instance);
    handle->flushQueue();
}

static void
fwd_callback(lcb_t instance, const void *cookie, lcb_error_t err,
    lcb_PKTFWDRESP *resp)
{
    Command *cmd = (Command *)cookie;
    if (err == LCB_SUCCESS) {
        cmd->getOrigin()->gotResponse(cmd, resp);
    } else {
        cmd->getOrigin()->gotResponse(cmd, err);
    }
    cmd->setReceived();
    cmd->maybeDestroy();
}

static void
flushed_callback(lcb_t instance, const void *cookie)
{
    Command *cmd = (Command *)cookie;
    cmd->maybeDestroy();
}

}


LCBHandle::LCBHandle(const string& host, const std::string& bucket, struct ev_loop *loop)
{
    lcb_create_st cropts;
    lcb_create_io_ops_st io_cropts;

    cropts.version = 2;
    cropts.v.v2.bucket = bucket.c_str();
    cropts.v.v2.host = host.c_str();

    io_cropts.v.v0.cookie = loop;
    io_cropts.v.v0.type = LCB_IO_OPS_LIBEV;
    lcb_create_io_ops(&io, &io_cropts);
    cropts.v.v2.io = io;
    lcb_create(&instance, &cropts);
    lcb_set_cookie(instance, this);
    lcb_set_bootstrap_callback(instance, bootstrap_callback);
    lcb_set_pktfwd_callback(instance, fwd_callback);
    lcb_set_pktflushed_callback(instance, flushed_callback);
    lcb_connect(instance);
}
