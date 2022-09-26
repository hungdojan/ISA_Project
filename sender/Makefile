CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic
# CFLAGS+=-O2 # Release
CFLAGS+=-g # Debug
LIBS=
TARGET=../dns_sender
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	$<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) *.o

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes $<

debug: $(TARGET)
	gdb -tui $<
