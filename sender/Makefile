CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -I../include -D_DEFAULT_SOURCE
# CFLAGS+=-O2 # Release
CFLAGS+=-g # Debug
LIBS=-lm
TARGET=../dns_sender
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
ARGS=-u 127.0.0.1 example.com data.txt ../examples/simple.txt

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	@$<	$(ARGS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) *.o

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes $< $(ARGS)

debug: $(TARGET)
	gdb -tui $<
