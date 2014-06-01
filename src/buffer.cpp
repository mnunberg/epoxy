#include "buffer.h"
using namespace Epoxy;
using std::vector;
using std::list;

Buffer::Buffer(size_t n)
{
    ndata = 0;
    refcount = 0;
    allocated = n;
    offset = 0;
    data = new char[allocated];
}

Buffer::~Buffer()
{
    delete[] data;
}

void
Buffer::clear()
{
    ndata = 0;
    refcount = 0;
    offset = 0;
}

void
Buffer::consumed(size_t n)
{
    offset += n;
    ndata -= n;
}

size_t
Buffer::added(size_t n)
{
    n = std::min(n, capacity());
    ndata += n;
    return n;
}

void
Buffer::tryRelase(BufferPool *pool)
{
    if (ndata > 0) {
        return;
    }
    if (!refcount) {
        pool->put(this);
    }
}

BufferPool::BufferPool()
{
    itemCapacity = 32;
    itemBlockSize = 4096;
}

BufferPool::~BufferPool()
{
    while (!buffers.empty()) {
        buffers.back()->tryRelase(this);
        buffers.pop_back();
    }
}

Buffer *
BufferPool::get(size_t n)
{
    if (!n) {
        n = itemBlockSize;
    }
    if (n != itemBlockSize || buffers.empty()) {
        return new Buffer(n);
    }

    while (!buffers.empty()) {
        Buffer *b = buffers.back();
        buffers.pop_back();
        if (b->allocated != itemBlockSize) {
            delete b;
        } else {
            b->clear();
            return b;
        }
    }
    // List is empty?
    return get(n);
}

void
BufferPool::put(Buffer *b)
{
    b->clear();
    if (b->allocated != itemBlockSize) {
        delete b;
    } else  if (buffers.size() >= itemCapacity) {
        delete b;
    } else {
        buffers.push_back(b);
    }
}


Rope::Rope(BufferPool *pool)
{
    bp = pool;
}

typedef list<Buffer*>::iterator BIter;
bool
Rope::getContig(char *s, size_t n)
{
    BIter ii = bufs.begin();
    for (; ii != bufs.end() && n > 0; ++ii) {
        // Copy the data
        size_t toCopy = std::min(n, (*ii)->size());
        if (toCopy) {
            memcpy(s, (*ii)->data, toCopy);
            n -= toCopy;
            s += toCopy;
        }
    }
    return n == 0;
}

void
Rope::getFrags(size_t n, vector<Buffer*>& ubufs, vector<iovec>& uiov)
{
    BIter ii = bufs.begin();
    for (; ii != bufs.end() && n > 0; ++ii) {
        size_t toAssign = std::min(n, (*ii)->size());
        if (toAssign) {
            iovec iovcur;
            iovcur.iov_base = (*ii)->data + (*ii)->offset;
            iovcur.iov_len = toAssign;
            uiov.push_back(iovcur);
            ubufs.push_back(*ii);
        }
    }
}

void
Rope::getReadBuffers(ReadContext& ctx)
{
    Buffer *lastBuf = bufs.back();
    iovec *iov_p;
    Buffer **buf_p;

    size_t nremaining;

    if (lastBuf->capacity() > 0) {
        ctx.iov[0].iov_base = lastBuf->data + lastBuf->offset;
        ctx.iov[0].iov_len = lastBuf->capacity();
        ctx.bk[0] = lastBuf;
        nremaining = EPOXY_NREAD_IOV - 1;
        iov_p = &ctx.iov[1];
        buf_p = &ctx.bk[1];
        ctx.chopFirst = true;

    } else {
        nremaining = EPOXY_NREAD_IOV;
        iov_p = &ctx.iov[0];
        buf_p = &ctx.bk[0];
    }
    for (unsigned ii = 0; ii < nremaining; ++ii) {
        Buffer *buf = bp->get();
        iov_p[ii].iov_base = buf->data;
        iov_p[ii].iov_len = buf->capacity();
        buf_p[ii] = buf;
    }
}

void
Rope::setReadAdded(ReadContext& ctx, size_t n)
{
    unsigned ii;
    for (ii = 0; ii < EPOXY_NREAD_IOV && n > 0; ii++) {
        Buffer *buf = ctx.bk[ii];
        n -= buf->added(n);
        if (ctx.chopFirst == false || ii > 0) {
            bufs.push_back(buf);
        }
    }
    for (; ii < EPOXY_NREAD_IOV; ii++) {
        bp->put(ctx.bk[ii]);
    }
}

void
Rope::consumed(size_t n)
{
    while(bufs.empty() == false && n > 0) {
        Buffer *buf = bufs.front();
        size_t toConsume = std::min(n, buf->size());
        buf->consumed(toConsume);
        if (buf->empty()) {
            bufs.pop_front();
            buf->tryRelase(bp);
        }
        n -= toConsume;
    }
}

size_t
Rope::size()
{
    size_t ret = 0;
    for (BIter ii = bufs.begin(); ii != bufs.end(); ii++) {
        ret += (*ii)->size();
    }
    return ret;
}
