#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <boost/foreach.hpp>
#include <gtest/gtest.h>

#include "iovec.h"

// TODO: FIXME build on this when you have time to focus !!!!!!!!!!1
typedef std::vector<size_t> svec_t;

class TestP : public ::testing::TestWithParam<svec_t>
{
};

TEST_P(TestP,Foo)
{
    svec_t const& p = GetParam();
}

class TestZero : public ::testing::Test
{
    const size_t buflen = 0x100000;
    char* buf; 
    static int fd;
public:
    void SetUp();
    void TearDown();
    void testlen(size_t iov_len);
};

int TestZero::fd = open("/dev/zero",O_RDONLY);

void TestZero::SetUp()
{
    buf = new char[buflen];

    assert(fd!=-1);
}

void TestZero::TearDown()
{
    delete[] buf;
}

void TestZero::testlen(size_t iov_len)
{
    const size_t count = buflen/iov_len;
    if (count > IoVec::maxiov)
    {
        ASSERT_THROW( IoVec(buf,buflen,iov_len), std::runtime_error);
        return;
    }

    IoVec vec(buf,buflen,iov_len);

    EXPECT_EQ(vec.size(),count);
    EXPECT_LE(vec.span(),buflen);
    EXPECT_EQ(vec.span(),vec.size()*iov_len);
    ASSERT_EQ(vec.base(),buf);
    ASSERT_LE(vec.base(count-1)-vec.base(),buflen);
    ASSERT_LE(vec.nbytes(),buflen);

    memset(buf,'a',buflen);

    ssize_t n = readv(fd,vec.front(),vec.size());

    ASSERT_NE(n,-1) << "size " << iov_len << " errno " << errno;
    EXPECT_EQ(n,iov_len*count);
}

TEST_F(TestZero,4096) { testlen(4096); }
TEST_F(TestZero,3333) { testlen(3333); }
TEST_F(TestZero,1024) { testlen(1024); }
TEST_F(TestZero,512) { testlen(512); }
TEST_F(TestZero,8) { testlen(8); }
TEST_F(TestZero,4) { testlen(4); }
TEST_F(TestZero,2) { testlen(2); }
TEST_F(TestZero,1) { testlen(1); }

