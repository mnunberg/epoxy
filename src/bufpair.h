#ifndef LCBPROXY_BUFPAIR_H
#define LCBPROXY_BUFPAIR_H
#include "common.h"
namespace Epoxy {

class BufPair {
public:
    enum Buftype {
        BACKBUF,
        COPY,
        STATIC
    };

    BufPair(const char *s, size_t n, Buftype = COPY);
    BufPair(lcb_IOV& iov, lcb_BACKBUF bk);
    size_t consumed(size_t total);
    bool empty() const { return iov.iov_len == 0; }
    void release();
    void fill(iovec *iovout) {
        iovout->iov_len = iov.iov_len;
        iovout->iov_base = iov.iov_base;
    }
private:
    union {
        lcb_BACKBUF bk;
        char *buf;
    };

    lcb_IOV iov;
    Buftype type;
};

} //namespace

#endif
