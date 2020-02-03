#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <cassert>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <gtest/gtest.h>

#include "iovec.h"

struct IoVecParams
{
    const size_t len, num_chans, chan;

    IoVecParams(size_t len, size_t num_chans=1, size_t chan=0)
    : len(len), num_chans(num_chans), chan(chan)
    {
        assert(chan < num_chans);
    };

    std::string str()
    {
        std::ostringstream ss;
        ss << "len " << len << " chan " << chan << " num_chans " << num_chans;
        return ss.str();
    };

};

struct BufferMgrParams
{
    char* const base;
    const size_t size;
    const size_t iov_len;
    const size_t num_chans;
    BufferMgrParams(char* base, size_t size, size_t iov_len, size_t num_chans=1)
    : base(base),
      size(size),
      iov_len(iov_len),
      num_chans(num_chans)
    {
    };
};

class IovecTester : public ::testing::TestWithParam<IoVecParams>
{
public:
    char* buf;
    static int fd;
    const size_t buflen = 0x100000;
    void SetUp() { buf = new char[buflen]; };
    void TearDown() { delete[] buf; };
};

size_t runlen(char* ptr, const char& c, size_t len)
{
    char* p = ptr;
    size_t ret = 0;
    do
    {
        ret = p-ptr;
        if (ret == len)
        {
            break;
        }
    } while(*p++ == c);
    return ret;
}

TEST_P(IovecTester,basic)
{
    IoVecParams p = GetParam();
    const size_t count = buflen/p.len/p.num_chans;
    BufferMgr bm(buf,buflen,p.len,p.num_chans);
 
    if (bm.num_units() > IoVec::maxiov)
    {
        ASSERT_THROW(IoVec(bm,p.chan), std::runtime_error);
        return;
    }

    char *ptr =bm.base(p.chan);
    IoVec vec(bm,p.chan);

    EXPECT_EQ(vec.size(),count) << p.str();
    ASSERT_EQ(vec.base(),bm.base(p.chan)) << p.str() << " buf " << static_cast<void*>(buf); 
    ASSERT_LE(vec.base(count-1)-vec.base(),buflen) << p.str();
    ASSERT_LE(vec.nbytes(),vec.iov_len()*vec.size()) << p.str();

    memset(buf,'a',buflen);

    ssize_t n = readv(fd,vec.front(),vec.size());

    ASSERT_NE(n,-1) << "size " << p.len << " errno " << errno;
    EXPECT_EQ(n,p.len*vec.size());

    if (p.num_chans == 1)
    {
        ASSERT_EQ(n,runlen(bm.base(p.chan),0,buflen));
    }
    else
    {
        char* base = bm.base(p.chan);
        for(size_t i=0; i<vec.size(); i++)
        {
            char *ptr = base + i*bm.m_stride;
            ASSERT_EQ(p.len,runlen(ptr,0,buflen));
        }
    }
};

// TODO: use netcat or similar
int IovecTester::fd = open("/dev/zero",O_RDONLY);

class BufferMgrTester : public ::testing::TestWithParam<BufferMgrParams>
{
};

TEST_P(BufferMgrTester,basic)
{
    BufferMgrParams p = GetParam();

    BufferMgr buf(p.base, p.size, p.iov_len, p.num_chans);

    ASSERT_EQ(buf.iov_len(),p.iov_len);
    ASSERT_EQ(buf.size(),p.size);
    ASSERT_EQ(buf.m_stride,p.num_chans*p.iov_len);
    ASSERT_EQ(buf.m_num_units,p.size/p.iov_len/p.num_chans);

    for(size_t chan=0; chan<p.num_chans; chan++)
    {
        ASSERT_EQ(buf.base(chan),p.base+chan*p.iov_len);
    }
}


#if 1
INSTANTIATE_TEST_SUITE_P(sizes,BufferMgrTester,
    testing::Values(
        BufferMgrParams(0,0x10000,0x100,1),
        BufferMgrParams(0,0x10000,0x100,4),
        BufferMgrParams(0,0x10000,123,3)
        ));
#endif

#if 1
INSTANTIATE_TEST_SUITE_P(sizes,IovecTester,
    testing::Values(
           IoVecParams(0x10000),
           IoVecParams(12345),
           IoVecParams(0x1000),
           IoVecParams(0x10)
           ));
#endif

#if 1
INSTANTIATE_TEST_SUITE_P(channels,IovecTester,
    testing::Values(
           IoVecParams(0x1000,1),
           IoVecParams(0x1000,2),
           IoVecParams(1234,4),
           IoVecParams(1234,4,1),
           IoVecParams(1234,4,2),
           IoVecParams(1234,4,3)
           ));
#endif

