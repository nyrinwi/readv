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

class TestF : public ::testing::Test
{
    const size_t buflen = 0x100000;
    char* buf; 
    static int fd;
public:
    void SetUp();
    void TearDown();
    void testlen(size_t iov_len);
};

int TestF::fd = open("/dev/zero",O_RDONLY);

void TestF::SetUp()
{
    buf = new char[buflen];
    // TODO: check this memset(buf,'a',buflen);

    assert(fd!=-1);
}

void TestF::TearDown()
{
    delete[] buf;
}

void TestF::testlen(size_t iov_len)
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

    ssize_t n = readv(fd,vec.front(),vec.size());

    ASSERT_NE(n,-1) << "size " << iov_len << " errno " << errno;
    EXPECT_EQ(n,iov_len*count);
}

TEST_F(TestF,3333) { testlen(3333); }
TEST_F(TestF,1024) { testlen(1024); }
TEST_F(TestF,512) { testlen(512); }
TEST_F(TestF,256) { testlen(256); }
TEST_F(TestF,128) { testlen(128); }
TEST_F(TestF,64) { testlen(64); }
TEST_F(TestF,32) { testlen(32); }
TEST_F(TestF,16) { testlen(16); }
TEST_F(TestF,8) { testlen(8); }
TEST_F(TestF,4) { testlen(4); }
TEST_F(TestF,2) { testlen(2); }
TEST_F(TestF,1) { testlen(1); }

