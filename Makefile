version=0.01

name=lua-iso8583-parser
dist=$(name)-$(version)

LUA_VERSION = 5.1
TARGET = iso8583.so


## Linux/BSD
CC = gcc
PREFIX ?=          /usr/local
PREFIX = /usr/local
CFLAGS =  -O3 -Wall -pedantic -DNDEBUG -g

CISO8583_CFLAGS =      -fpic
CISO8583_LDFLAGS =     -shared 
LUA_INCLUDE_DIR ?= $(PREFIX)/include
LUA_LIB_DIR ?=     $(PREFIX)/lib/lua/$(LUA_VERSION)
LUA_INCLUDE_DIR ?= $(PREFIX)/include
LUA_CMODULE_DIR ?= $(PREFIX)/lib/lua/$(LUA_VERSION)
LUA_MODULE_DIR ?=  $(PREFIX)/share/lua/$(LUA_VERSION)
LUA_BIN_DIR ?=     $(PREFIX)/bin

BUILD_CFLAGS =  `pkg-config --cflags --libs lua-$(LUA_VERSION)` $(CISO8583_CFLAGS)
OBJS = iso8583.o luaiso8583.o


INSTALL ?= install

.PHONY: all clean dist test t

all: $(TARGET)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) -o $@ $<


$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $(CISO8583_LDFLAGS) -o $@ $(OBJS)


test: all

install: $(TARGET)
	mkdir -p $(DESTDIR)$(LUA_CMODULE_DIR)
	rm -f $(DESTDIR)$(LUA_CMODULE_DIR)/$(TARGET)
	cp $(TARGET) $(DESTDIR)$(LUA_CMODULE_DIR)
	chmod $(EXECPERM) $(DESTDIR)$(LUA_CMODULE_DIR)/$(TARGET)

clean:
	rm -f  *.so $(TARGET) $(OBJS)

