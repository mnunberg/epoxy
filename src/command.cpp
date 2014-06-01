#include "command.h"
#include "buffer.h"
using namespace Epoxy;
using std::vector;
using std::list;


Command::Command(const RequestHeader& hdr)
{
    origin = NULL;
    target = NULL;
    opaque = hdr.request.opaque;
    opcode = hdr.request.opcode;
}

SmallCommand::SmallCommand(const RequestHeader& hdr, Rope* rope) :
        Command(hdr)
{
    packetSize = sizeof hdr.bytes;
    packetSize += ntohl(hdr.request.bodylen);
    rope->getContig(buf, packetSize);
    rope->consumed(packetSize);
}

void
SmallCommand::makeLcbBuf(lcb_CMDPKTFWD& cmd)
{
    cmd.vb.vtype = LCB_KV_COPY;
    cmd.vb.u_buf.contig.bytes = buf;
    cmd.vb.u_buf.contig.nbytes = packetSize;
}


// PayloadCommand
PayloadCommand::PayloadCommand(const RequestHeader& hdr, Rope* rope) :
        Command(hdr)
{
    size_t pktsize = ntohl(hdr.request.bodylen) + sizeof hdr.bytes;
    rope->getFrags(pktsize, buffers, iov);
    flushed = false;
}

void
PayloadCommand::makeLcbBuf(lcb_CMDPKTFWD& cmd)
{
    cmd.vb.vtype = LCB_KV_CONTIG;
    cmd.vb.u_buf.multi.iov = (lcb_IOV *)&iov[0];
    cmd.vb.u_buf.multi.niov = iov.size();
}
