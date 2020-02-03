# vim: noet
#
CC=g++
GTEST=$(HOME)/src/googletest-release-1.10.0/googletest

CXXFLAGS+=-I$(GTEST)/include -std=c++11 -ggdb
LDLIBS=-L$(GTEST)/lib -lgtest_main -lgtest -lpthread
LDFLAGS+=-ggdb

all:: test_readv test

test_readv: test_readv.o iovec.o

test:
	./test_readv

.PHONY: tags
tags:
	ctags *.cpp *.h

clean:
	$(RM) test_readv *.o

