# vim: noet
#
CC=g++
GTEST=$(HOME)/src/googletest-release-1.8.1/googletest

CXXFLAGS+=-I$(GTEST)/include -std=c++11
LDLIBS=-L$(GTEST) -lgtest_main -lgtest -lpthread

all:: test_readv
	./test_readv

test_readv: test_readv.o iovec.o

clean:
	$(RM) test_readv *.o
