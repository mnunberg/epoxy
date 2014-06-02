#include "bufpair.h"
using namespace Epoxy;

BufPair::BufPair(const char *s, size_t n, Buftype type)
{
    this->type = type;
    assert(type != BACKBUF);
    if (type == COPY) {
        buf = new char[n];
        memcpy(buf, s, n);
    } else {
        buf = (char*)s;
    }

    iov.iov_base = buf;
    iov.iov_len = n;
}

BufPair::BufPair(lcb_IOV& iov, lcb_BACKBUF bk)
{
    this->bk = bk;
    this->iov = iov;
    this->type = BACKBUF;
}

size_t
BufPair::consumed(size_t n)
{
    n = std::min(n, iov.iov_len);
    char *nextp = (char *)iov.iov_base;
    nextp += n;
    iov.iov_base = (void *)nextp;
    iov.iov_len -= n;
    return n;
}

void
BufPair::release()
{
    if (type == BACKBUF) {
        lcb_backbuf_unref(bk);
    } else if (type == COPY) {
        delete[] buf;
    }
}
