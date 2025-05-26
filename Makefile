GCC=gcc

GCCFLAGS=-Wall -fdiagnostics-color=always -pedantic

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

all: before $(EXEC)

tests: before $(EXEC_TESTS)
	$(EXEC_TESTS)

before:
	mkdir -p bin

$(EXEC):
	$(GCC) $(GCCFLAGS) -I./src/utils/headers $(UTILS_SRC) $(MAIN_SRC) -o $@

$(EXEC_TESTS):
	$(GCC) $(GCCFLAGS) -I./src/utils/headers $(UTILS_SRC) $(TESTS_SRC) -o $@

clean:
	rm -rf bin

.PHONY: all clean tests

