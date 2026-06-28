CC ?= cc
CXX ?= c++
AR ?= ar
CFLAGS ?= -O1 -g -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function
CXXFLAGS ?= -O1 -g -fno-omit-frame-pointer -Wall -Wextra
SAN ?=
CPPFLAGS += -Isrc
CFLAGS += $(SAN)
CXXFLAGS += $(SAN)
LDFLAGS += $(SAN)
SRC := $(shell find src -name '*.c' ! -path 'src/cli/relaymesh.c' | sort)
OBJ := $(SRC:.c=.o)
TEST_SRC := $(shell find tests -name 'test_*.c' | sort)
TEST_BIN := $(TEST_SRC:.c=)
HARNESS_SRC := $(shell find fuzz -name '*_fuzzer.cc' | sort)
HARNESS_BIN := $(HARNESS_SRC:.cc=)

all: librelaymesh.a relaymesh $(HARNESS_BIN)

librelaymesh.a: $(OBJ)
	$(AR) rcs $@ $(OBJ)

relaymesh: src/cli/relaymesh.o librelaymesh.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%: %.c librelaymesh.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< librelaymesh.a $(LDFLAGS)

%_fuzzer: %_fuzzer.cc librelaymesh.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< librelaymesh.a $(LDFLAGS)

test: $(TEST_BIN)
	@set -e; for t in $(TEST_BIN); do $$t; done

seeds:
	python3 tools/generate_seeds.py

clean:
	find src tests fuzz -name '*.o' -delete
	rm -f librelaymesh.a relaymesh $(TEST_BIN) $(HARNESS_BIN)

.PHONY: all test clean seeds
