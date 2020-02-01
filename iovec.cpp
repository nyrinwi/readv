#include <sstream>
#include <stdexcept>
#include <cassert>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <boost/foreach.hpp>
#include "iovec.h"

const long  IoVec::maxiov = sysconf(_SC_IOV_MAX);

IoVec::IoVec(char* base, size_t nbytes, size_t iov_len, size_t nchans, size_t chan)
{
    char* ptr=base;
    const size_t stride = nchans*iov_len;
    const size_t len = nbytes/stride;
    assert(len);

    if (maxiov != -1 and static_cast<long>(len) > maxiov)
    {
        std::ostringstream ss;
        ss << "iov too long " << len << " > " << maxiov;
        throw std::runtime_error(ss.str());
    }
    m_vec.resize(len);

    BOOST_FOREACH(struct iovec& v, m_vec)
    {
        v.iov_base = ptr;
        v.iov_len = iov_len;
        ptr += stride;
    }
    assert((ptr-base)<=nbytes);
}
