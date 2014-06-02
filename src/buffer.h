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
    /**
     * Creates a new buffer object
     * @param n The capacity of the buffer
     */
    Buffer(size_t n, BufferPool *pool);
    ~Buffer();

    /**
     * Get the current capacity of the buffer
     * @return The maximum number of bytes which may be written to the buffer
     */
    size_t capacity() const { return allocated - (offset+ndata); }

    /**
     * Get the size of used data in the buffer
     * @return The number of bytes which may be read from the buffer
     */
    size_t size() const { return ndata; }

    /** @brief equivalent to size() == 0 */
    bool empty() const { return ndata == 0; }

    /**
     * Obtain a pointer to the first byte in the buffer. This buffer may be
     * used for reading data up to size() bytes
     * @return A pointer to the first used byte
     */
    char *getReadHead() { return data + offset; }

    /**
     * Obtain a pointer to the first unused byte in the buffer. This buffer
     * may be used for adding data up to capacity() bytes.
     * @return a pointer to the first unused byte.
     */
    char *getWriteHead() { return data + offset + ndata; }

    /**
     * Indicate that bytes in the buffer are no longer required. This will
     * have the effect of consuming the specified number of bytes (from the
     * beginning of the buffer) so that subsequent reads will not return
     * these bytes.
     *
     * @param n The number of bytes no longer required
     */
    void consumed(size_t n);

    /**
     * Indicate that data has been added to the buffer
     * @param the total number of bytes which may have been added to the buffer
     * @return The number of actual bytes added to the buffer
     * @note This method is intended to be used if reading into an iovec-like
     * structure where there are multiple Buffer objects. In such a scenario
     * the socket recv() routine only reports the total number of bytes received.
     * The intended use case is to traverse all the elements in the iovec array,
     * calling each element's corresponding Buffer::added() function and
     * decrementing the total number of bytes read by the return value from this
     * method.
     */
    size_t added(size_t nw);

    /** Clear the buffer. This resets all offsets */
    void clear();
    void ref() { refcount++; }
    void unref();
    void fillIovec(iovec& iov) { iov.iov_base = getWriteHead(); iov.iov_len = capacity(); }

private:
    size_t ndata;
    size_t refcount;
    size_t offset;
    size_t allocated;
    char *data;
    BufferPool *parent;
    friend class BufferPool;
};

struct ReadContext {
    ReadContext() { chopFirst = false; }
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
