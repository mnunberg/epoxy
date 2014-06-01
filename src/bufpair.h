#ifndef LCBPROXY_BUFPAIR_H
#define LCBPROXY_BUFPAIR_H
#include "common.h"
namespace Epoxy {

class BufPair {
public:
    BufPair(const char *s, size_t n);
    BufPair(lcb_IOV& iov, lcb_BACKBUF bk);
    size_t consumed(size_t total);
    bool empty() const { return iov.iov_len == 0; }
    void release();
    void fill(iovec *iovout) {
        iovout->iov_len = iov.iov_len;
        iovout->iov_base = iov.iov_base;
    }
private:
    lcb_BACKBUF bk;
    lcb_IOV iov;
    bool isMalloced;
};

} //namespace

#endif
