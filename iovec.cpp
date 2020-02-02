#include <sstream>
#include <stdexcept>
#include <cassert>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <boost/foreach.hpp>
#include "iovec.h"

const long  IoVec::maxiov = sysconf(_SC_IOV_MAX);

IoVec::IoVec(const BufferMgr &buf, size_t chan)
: m_buf(buf)
{
    char* ptr=buf.base(chan);

    if (maxiov != -1 and static_cast<long>(buf.num_units()) > maxiov)
    {
        throw std::runtime_error("iov too long");
    }
    m_vec.resize(buf.num_units());

    BOOST_FOREACH(struct iovec& v, m_vec)
    {
        const size_t i = &v - &m_vec[0];
        v.iov_base = buf.base(chan,i);
        v.iov_len = buf.iov_len();
    }
}

BufferMgr::BufferMgr(char* base, size_t size, size_t iov_len, size_t nchans)
: m_base(base),
  m_size(size),
  m_iov_len(iov_len),
  m_nchans(nchans)
{
    assert(iov_len);
    m_stride = m_nchans*m_iov_len;
    m_num_units = m_size/m_stride;
    assert(m_num_units);
}

char* BufferMgr::base(size_t channel, size_t unit) const
{
    assert(channel < m_nchans);
    assert(unit < m_num_units);
    char *ret = m_base + unit*m_stride + channel*m_iov_len;
    return ret;
}


