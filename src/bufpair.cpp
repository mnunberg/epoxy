#include "bufpair.h"
using namespace Epoxy;

BufPair::BufPair(const char *s, size_t n)
{
    isMalloced = true;
    bk = (lcb_BACKBUF) malloc(n);
    memcpy((void *)bk, s, n);
    iov.iov_base = (void*)bk;
    iov.iov_len = n;
}

BufPair::BufPair(lcb_IOV& iov, lcb_BACKBUF bk)
{
    this->bk = bk;
    this->iov = iov;
    isMalloced = false;
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
    if (isMalloced) {
        free(bk);
    } else {
        lcb_backbuf_unref(bk);
    }
}
