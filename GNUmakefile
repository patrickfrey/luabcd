# Build flavor parameters:
ifeq ($(strip $(COMPILER)),clang)
CC=clang
else ifeq ($(strip $(COMPILER)),gcc)
CC=g++
else
CC=clang
endif
ifeq ($(strip $(CC)),clang)
DEBUGFLAGS:=-fstandalone-debug
endif
AR=ar rcs
LNKSO=$(CC) -shared

ifeq ($(strip $(RELEASE)),)
DEBUGFLAGS:=-ggdb -g3 -O0 $(DEBUGFLAGS)
else
DEBUGFLAGS=-O3
endif
ifneq ($(strip $(VERBOSE)),)
CXXVBFLAGS 	:=-v
TSTVBFLAGS 	:=-V
endif

ifeq ($(wildcard Lua.inc),)
$(error File Lua.inc not found. Please run ./configure first!)
endif

include Lua.inc

# Project settings:
BUILDDIR := build
DESTINATION := $(PREFIX)/bin
MAKEDEP  := Lua.inc GNUmakefile configure
SRCDIR   := src
TESTDIR  := tests
DOCDIR   := doc
STDFLAGS := -std=c++17
CXXFLAGS := -c $(STDFLAGS) $(CXXVBFLAGS) $(DEBUGFLAGS) -fPIC -Wall -Wshadow -pedantic -Wfatal-errors -fvisibility=hidden -pthread
INCFLAGS := -I$(SRCDIR) -I$(LUAINC)
LDFLAGS  := -g -pthread
LDLIBS   := -lm -lstdc++
LIBOBJS  := $(BUILDDIR)/bcd.o
MODOBJS  := $(BUILDDIR)/lualib_bcd.o
MODULE   := $(BUILDDIR)/bcd.so

# Build targets:
all : build $(LIBOBJS) $(MODULE) $(MAKEDEP)

clean: build
	rm $(BUILDDIR)/* .depend
build:
	mkdir -p $(BUILDDIR)

# Generate include dependencies:
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
depend: .depend
.depend: $(SOURCES)
	$(CC) $(STDFLAGS) $(INCFLAGS) -MM $^ | sed -E 's@^([^: ]*[:])@\$(BUILDDIR)/\1@' > .depend
include .depend

# Build rules:
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(MAKEDEP)
	$(CC) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

$(MODULE): $(LIBOBJS) $(MODOBJS)
	$(LNKSO) $(LUALIBS) $(LDLIBS) -o $@ $(MODOBJS) $(LIBOBJS)

test : all
	tests/luatests.sh "$(LUABIN)"
check: test

install: all
	cp $(MODULE) $(CMODPATH)/

