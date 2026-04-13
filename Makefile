CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2 -Iinclude
LDFLAGS =

SRCS = src/flux-disasm.c src/symbol_table.c src/relocation.c src/output.c src/xref.c
OBJS = $(SRCS:.c=.o)
TARGET = flux-disasm

TEST_TARGET = test_disasm

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/test_disasm.c $(SRCS)
	$(CC) $(CFLAGS) -o $@ tests/test_disasm.c $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)
