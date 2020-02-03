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

struct Params
{
    const size_t len, nchans, chan;
    Params(size_t len, size_t nchans=1, size_t chan=0)
    : len(len), nchans(nchans), chan(chan) {};
};

class TestP : public ::testing::TestWithParam<Params>
{
public:
    char* buf;
    static int fd;
    const size_t buflen = 0x100000;
    TestP() {};
    ~TestP() {};
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

TEST_P(TestP,Foo)
{
    Params p = GetParam();
    const size_t count = buflen/p.len/p.nchans;
    BufferMgr bm(buf,buflen,p.len,p.nchans);
 
    if (bm.num_units() > IoVec::maxiov)
    {
        ASSERT_THROW(IoVec(bm,p.chan), std::runtime_error);
        return;
    }

    char *ptr =bm.base();
    IoVec vec(bm,p.chan);

    EXPECT_EQ(vec.size(),count);
    ASSERT_EQ(vec.base(),buf);
    ASSERT_LE(vec.base(count-1)-vec.base(),buflen);
    ASSERT_LE(vec.nbytes(),buflen);

    memset(buf,'a',buflen);

    ssize_t n = readv(fd,vec.front(),vec.size());

    ASSERT_NE(n,-1) << "size " << p.len << " errno " << errno;
    EXPECT_EQ(n,p.len*count);
    ASSERT_EQ(n, runlen(bm.base(p.chan),0,buflen));
};

// TODO: use netcat or similar
int TestP::fd = open("/dev/zero",O_RDONLY);

TEST(BufferTest,1)
{
    char* base = NULL;
    const size_t bufflen = 0x100000;
    const size_t iov_len = 123;
    const size_t nchans = 4;
    const size_t nunits = bufflen/(nchans*iov_len);
    BufferMgr buf(NULL, bufflen, iov_len, nchans);
    ASSERT_EQ(buf.iov_len(),iov_len);
    ASSERT_EQ(buf.size(),bufflen);
    ASSERT_EQ(buf.m_stride,nchans*iov_len);
    ASSERT_EQ(buf.m_num_units,nunits);
    ASSERT_EQ(buf.base(0),base);
    ASSERT_EQ(buf.base(0,2),base+buf.m_stride*2);
    ASSERT_EQ(buf.base(1),base+iov_len);
    ASSERT_EQ(buf.base(1,2),base+iov_len+buf.m_stride*2);
}


#if 1
INSTANTIATE_TEST_SUITE_P(sizes,TestP,
    testing::Values(
           Params(0x10000),
           Params(12345),
           Params(0x1000),
           Params(0x10)));

INSTANTIATE_TEST_SUITE_P(channels,TestP,
    testing::Values(
           Params(0x1000,1),
           Params(0x1000,2),
           Params(0x1000,4)));
#endif

