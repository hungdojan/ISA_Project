CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -Werror -I../include
# CFLAGS+=-O2 # Release
CFLAGS+=-g # Debug
LIBS=-lm
TARGET=../dns_receiver
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
ARGS=example.com data

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	@$< $(ARGS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) *.o

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes $< $(ARGS)

debug: $(TARGET)
	gdb -tui $<
