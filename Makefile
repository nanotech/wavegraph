CC ?= cc
CFLAGS += -std=c99 -Wall -g
LIBS = -lpng

MODULES = list libpngio/pngio

include exe.mk
