CC ?= cc
CXX ?= c++
CFLAGS += -std=c99 -Wall -g
CXXFLAGS += -std=c++11 -stdlib=libc++ -Wall -g -O2
LIBS = -lpng -lfftw3

MODULES = libpngio/pngio spectrogram

include exe.mk
