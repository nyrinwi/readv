#pragma once
#include <cstdlib>
#include <vector>

typedef std::vector<struct iovec> iovec_t;

class IoVec
{
public:
    static const long  maxiov;
    iovec_t m_vec;
    IoVec(char* base, size_t nbytes, size_t iov_len, size_t gap=0);
    size_t size() { return m_vec.size(); };
    size_t iov_len() { return m_vec.front().iov_len; };
    size_t nbytes() { return size() * iov_len(); };
    size_t span() { return (base(size()-1) - base(0)) + iov_len();};
    char* base(size_t seg=0) { return reinterpret_cast<char*>(m_vec[seg].iov_base); };
    struct iovec* front() { return &m_vec[0]; };

    // TODO: implement me
    ssize_t readv(int fd, bool bounce=false);
};
