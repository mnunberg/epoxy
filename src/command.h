#ifndef LCBPROXY_COMMAND_H
#define LCBPROXY_COMMAND_H
#include "common.h"
#include <arpa/inet.h>
#include <memcached/protocol_binary.h>

namespace Epoxy {
class Client;
class LCBHandle;
class Buffer;

typedef protocol_binary_request_header RequestHeader;
typedef protocol_binary_response_header ResponseHeader;

class Command {
public:
    Command(const RequestHeader&);
    virtual ~Command() {}
    virtual void makeLcbBuf(lcb_CMDPKTFWD&) = 0;
    virtual void setFlushed() { }
    virtual void maybeDestroy() = 0;
    void setReceived() { received = true; }
    uint32_t getOpaque() const { return opaque; }
    uint8_t getOpcode() const { return opcode; }

    void setDispatched(Client *c, LCBHandle *h) { origin=c;target=h; }
    Client* getOrigin() { return origin; }
    LCBHandle* getTarget() { return target; }

protected:
    Client *origin; /**< Client which sent the command */
    LCBHandle *target; /**< Target object which is servicing this command */
    uint32_t opaque; /**< Original opaque */
    uint8_t opcode;
    bool received;
};


class SmallCommand : public Command {
public:
    SmallCommand(const RequestHeader&, Rope *rope);
    virtual void makeLcbBuf(lcb_CMDPKTFWD&);
    virtual void maybeDestroy() { if (received) { delete this; } }

private:
    char buf[EPOXY_SMALLCMD_MAXSZ];
    size_t packetSize;
};

class PayloadCommand : public Command {
public:
    PayloadCommand(const RequestHeader&, Rope *rope);
    virtual void makeLcbBuf(lcb_CMDPKTFWD&);
    virtual void maybeDestroy() { if (received && flushed) { delete this; } }
    virtual void setFlushed() { flushed = true; }

private:
    /** Whether this command has been send to the network */
    bool flushed;
    /** Buffers containing the payload+header for this command */
    std::vector<Buffer *>buffers;
    std::vector<iovec> iov;
};

static inline size_t
getHeaderValueSize(const RequestHeader& hdr) {
    uint32_t sz = ntohl(hdr.request.bodylen);
    uint8_t extlen = hdr.request.extlen;
    uint16_t klen = ntohs(hdr.request.keylen);
    return sz - (extlen + klen);
}

static inline size_t
getPacketSize(const RequestHeader& hdr)
{
    size_t sz = sizeof hdr.bytes;
    sz += ntohl(hdr.request.bodylen);
    return sz;
}

static inline void
makeResponseTemplate(ResponseHeader& res, const RequestHeader& req)
{
    res.response.opcode = req.request.opcode;
    res.response.magic = PROTOCOL_BINARY_RES;
    res.response.opaque = req.request.opaque;
}

} //namespace

#endif
