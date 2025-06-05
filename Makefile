GCC=gcc

GCCFLAGS=-Wall -fdiagnostics-color=always -pedantic -I./src/utils/headers

ifneq ($(DEBUG), 0)
GCCFLAGS += -g -fsanitize=address
else
GCCFLAGS += -O2
endif

TESTS_SRC = $(wildcard src/tests/*.c)
UTILS_SRC = $(wildcard src/utils/*.c)
MAIN_SRC = src/main.c
EXEC=./bin/main.out
EXEC_TESTS=./bin/tests.out

all: clean before $(EXEC)

tests: before $(EXEC_TESTS)
	$(EXEC_TESTS)

before:
	mkdir -p bin
	mkdir -p src/encodings

$(EXEC):
	$(GCC) $(GCCFLAGS) $(UTILS_SRC) $(MAIN_SRC) -o $@

$(EXEC_TESTS):
	$(GCC) $(GCCFLAGS) -I./src/tests/headers $(UTILS_SRC) $(TESTS_SRC) -o $@

clean:
	rm -rf bin
	rm -rf src/encodings

.PHONY: all clean tests

