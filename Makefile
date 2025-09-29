CC ?= cc
CFLAGS ?= -std=c17 -Wall -Wextra -I.

UNAME_S := $(shell uname -s 2>/dev/null)
LDFLAGS :=
ifeq ($(UNAME_S),Darwin)
LDFLAGS += -framework Cocoa
endif

TEST_SRCS := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/test_%.c,tests/test_%,$(TEST_SRCS))

.PHONY: all test clean docs stubs

all: stubs tests docs clean

tests/test_%: tests/test_%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests: $(TEST_BINS)
	@./tests/run_tests.sh

clean:
	@rm -rf $(TEST_BINS) $(STUB_BINS) tests/stubs/*.c.out tests/stubs/*.cpp.out tests/stubs/build

stubs:
	@if [ -x "./tests/build_stubs.sh" ]; then \
		sh ./tests/build_stubs.sh; \
	else \
		$(MAKE) stubs; \
	fi

docs:
	@./tests/build_docs.sh
