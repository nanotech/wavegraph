CC ?= cc
CXX ?= c++
CFLAGS += -std=c99 -Wall -g
CXXFLAGS += -std=c++11 -stdlib=libc++ -Wall -g
LIBS = -lpng

MODULES = libpngio/pngio

include exe.mk
