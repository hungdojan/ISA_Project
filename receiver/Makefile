CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -I../include
# CFLAGS+=-O2 # Release
CFLAGS+=-g # Debug
LIBS=
TARGET=../dns_receiver
SHARED=$(patsubst %.c,%.o,$(wildcard ../shared/*.c))
OBJS=$(patsubst %.c,%.o,$(wildcard *.c)) $(SHARED)
ARGS=example.com test_dir

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	@$< $(ARGS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(TARGET) *.o test_dir

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes $< $(ARGS)

debug: $(TARGET)
	gdb -tui $< $(ARGS)
