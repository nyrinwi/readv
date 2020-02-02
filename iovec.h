#pragma once
#include <cstdlib>
#include <vector>

typedef std::vector<struct iovec> iovec_t;

// Calculator for a multi-channel buffer
class BufferMgr
{
public:
    char* const m_base;
    const size_t m_size;
    const size_t m_iov_len;
    const size_t m_nchans;
    mutable size_t m_num_units;
    mutable size_t m_stride;

    BufferMgr(char* base, size_t size, size_t iov_len, size_t nchans=1);
    size_t size() const { return m_size; };
    size_t iov_len() const { return m_iov_len; };
    char* base(size_t channel=0, size_t unit=0) const;
    size_t span() const { return m_stride*m_num_units; };
    size_t num_units() const { return m_num_units; };
};

class IoVec
{
    BufferMgr m_buf;
public:
    static const long  maxiov;
    iovec_t m_vec;
    IoVec(const BufferMgr &buf, size_t chan=0);
    size_t size() { return m_vec.size(); };
    size_t iov_len() { return m_vec.front().iov_len; };
    size_t nbytes() { return size() * iov_len(); };
    size_t span() { return (base(size()-1) - base(0)) + iov_len();};
    char* base(size_t seg=0) { return reinterpret_cast<char*>(m_vec[seg].iov_base); };
    struct iovec* front() { return &m_vec[0]; };

    // TODO: implement me
    ssize_t readv(int fd, bool bounce=false);
};

