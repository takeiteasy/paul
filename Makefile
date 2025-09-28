# Simple Makefile to build tests

CC ?= cc
CFLAGS ?= -std=c17 -Wall -Wextra -I.

# Add macOS-specific linker flags when running on Darwin
UNAME_S := $(shell uname -s 2>/dev/null)
LDFLAGS :=
ifeq ($(UNAME_S),Darwin)
LDFLAGS += -framework Cocoa
endif

TEST_SRCS := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/test_%.c,tests/test_%,$(TEST_SRCS))

# Build stubs for each paul_*.h under tests/stubs
PAUL_HEADERS := $(wildcard paul_*.h)
STUB_C_SRCS := $(patsubst paul_%.h,tests/stubs/paul_%.c,$(PAUL_HEADERS))
STUB_CPP_SRCS := $(patsubst paul_%.h,tests/stubs/paul_%.cpp,$(PAUL_HEADERS))
STUB_C_BINS := $(patsubst %.c,%.c.out,$(STUB_C_SRCS))
STUB_CPP_BINS := $(patsubst %.cpp,%.cpp.out,$(STUB_CPP_SRCS))

STUB_BINS := $(STUB_C_BINS) $(STUB_CPP_BINS)

.PHONY: all test clean
all: test

tests/test_%: tests/test_%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TEST_BINS)
	@./tests/run_tests.sh

stubs: $(STUB_BINS)

# Compile C stubs
tests/stubs/%.c.out: tests/stubs/%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Compile C++ stubs (use C++ compiler if available, fall back to CC)
CXX ?= c++
tests/stubs/%.cpp.out: tests/stubs/%.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TEST_BINS) $(STUB_BINS)

.PHONY: build-stubs clean-all
build-stubs: $(STUB_BINS)

clean-all: clean
	rm -rf tests/stubs/*.c.out tests/stubs/*.cpp.out
