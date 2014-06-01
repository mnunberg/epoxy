#ifndef LCBPROXY_BUFFER_H
#define LCBPROXY_BUFFER_H
#include <cstdlib>
#include <vector>
#include "common.h"

// This is a miniature version of rdb_ROPE implemented in C++
// rather than C

namespace Epoxy {
class BufferPool;
class Buffer;

class BufferPool {
public:
    BufferPool();
    ~BufferPool();
    void put(Buffer*);
    Buffer* get(size_t n = 0);
private:
    std::vector<Buffer*> buffers;
    size_t itemCapacity;
    size_t itemBlockSize;
};

class Buffer {
public:
    Buffer(size_t n);
    ~Buffer();
    size_t capacity() const { return allocated - (offset+ndata); }
    size_t size() const { return ndata; }
    void consumed(size_t n);
    size_t added(size_t nw);
    void clear();
    void tryRelase(BufferPool *pool);
    bool empty() const { return ndata == 0; }

private:
    size_t ndata;
    size_t refcount;
    size_t offset;
    size_t allocated;
    char *data;
    friend class BufferPool;
    friend class Rope;
};

struct ReadContext {
    ReadContext() {
        chopFirst = false;
    }
    bool chopFirst;
    iovec iov[EPOXY_NREAD_IOV];
    Buffer *bk[EPOXY_NREAD_IOV];
};

class Rope {
public:
    Rope(BufferPool* pool);
    bool getContig(char *s, size_t n);
    void getFrags(size_t n, std::vector<Buffer*>& bufs, std::vector<iovec>& iov);
    void getReadBuffers(ReadContext&);
    void setReadAdded(ReadContext&, size_t n);
    void consumed(size_t n);
    size_t size();

private:
    std::list<Buffer *> bufs;
    BufferPool *bp;
};

}

#endif
