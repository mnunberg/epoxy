#ifndef LCBPROXY_COMMAND_H
#define LCBPROXY_COMMAND_H
#include "common.h"
#include <arpa/inet.h>
#include <memcached/protocol_binary.h>

namespace Epoxy {
typedef protocol_binary_request_header RequestHeader;
typedef protocol_binary_response_header ResponseHeader;

/**
 * Abstract base class for commands.
 * @see SmallCommand
 * @see PayloadCommand
 */
class Command {
public:
    /**
     * Initialize a new command object
     * @param hdr request header received
     */
    Command(const RequestHeader& hdr);
    virtual ~Command() {}

    /**
     * Initialize an lcb packet forwarding command
     * @param[out] cmd the command to initialize
     */
    virtual void makeLcbBuf(lcb_CMDPKTFWD& cmd) = 0;

    /**
     * Indicate that the data has been flushed to the target LCBHandle's network
     */
    virtual void setFlushed() { }

    /**
     * Call this after each event (flushed or received). This will free the
     * command object if no longer needed
     */
    virtual void maybeDestroy() = 0;

    /**
     * Indicate that the response for the command has been received
     */
    void setReceived() { received = true; }

    /**
     * Get the original opaque identifier for this command
     * @return
     */
    uint32_t getOpaque() const { return opaque; }

    /**
     * Get the original opcode for this command
     * @return
     */
    uint8_t getOpcode() const { return opcode; }

    /**
     * Indicate that this command has been dispatched to its destination
     * @param c The origin Client object on which the request was received
     * @param h The destination LCBHandle object which will service the request
     */
    void setDispatched(Client *c, LCBHandle *h) { origin=c;target=h; }

    /**
     * Get the origin client on which the request was received
     * @return
     */
    Client* getOrigin() const { return origin; }

    /**
     * Get the destination LCBHandle object which is servicing this request
     * @return
     */
    LCBHandle* getTarget() { return target; }

protected:
    Client *origin; /**< Client which sent the command */
    LCBHandle *target; /**< Target object which is servicing this command */
    uint32_t opaque; /**< Original opaque */
    uint8_t opcode;
    bool received;
};

/**
 * Class for small commands with a small payload. These commands are dispatched
 * to their destination by copying over the buffers. The total packet size
 * for this command must not exceed `EXPOXY_SMALLCMD_MAXSZ`
 */
class SmallCommand : public Command {
public:
    /**
     * Create a new command.
     * @param The header object
     * @param rope The rope object containing the received buffers. The Rope
     * object's position will be advanced
     */
    SmallCommand(const RequestHeader& hdr, Rope *rope);
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
    virtual void maybeDestroy();
    virtual void setFlushed() { flushed = true; }

private:
    /** Whether this command has been send to the network */
    bool flushed;
    /** Buffers containing the payload+header for this command */
    std::vector<Buffer *>buffers;
    std::vector<iovec> iov;
};

/**
 * Convenience function to retrieve the value size of the packet. The value
 * size is the key's value for the payload (i.e. the packet size, minus the
 * key size, header size, and extras)
 * @param hdr The header
 * @return value size
 */
static inline size_t
getHeaderValueSize(const RequestHeader& hdr) {
    uint32_t sz = ntohl(hdr.request.bodylen);
    uint8_t extlen = hdr.request.extlen;
    uint16_t klen = ntohs(hdr.request.keylen);
    return sz - (extlen + klen);
}

/**
 * Convenience function to return the packet's total size
 * @param hdr The header
 * @return The total packet size
 */
static inline size_t
getPacketSize(const RequestHeader& hdr)
{
    size_t sz = sizeof hdr.bytes;
    sz += ntohl(hdr.request.bodylen);
    return sz;
}

/**
 * Initialize a response with its basic fields. This will initialize the
 * response based on the request's opcode and opaque values
 * @param[out] res The response to initialize
 * @param[in] req The request being serviced
 */
static inline void
makeResponseTemplate(ResponseHeader& res, const RequestHeader& req)
{
    res.response.opcode = req.request.opcode;
    res.response.magic = PROTOCOL_BINARY_RES;
    res.response.opaque = req.request.opaque;
}

} //namespace

#endif
