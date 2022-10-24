CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -I../include -D_DEFAULT_SOURCE
# CFLAGS+=-O2 # Release
CFLAGS+=-g # Debug
LIBS=
TARGET=../dns_sender
SHARED=$(patsubst %.c,%.o,$(wildcard ../shared/*.c))
OBJS=$(patsubst %.c,%.o,$(wildcard *.c)) $(SHARED)

ARGS=-u ::1 example.com ipv4.txt ../examples/tcp_data.txt
# ARGS=-u ::1 example.com ipv6.txt "~/disk/.Music/ZUTOMAYO - Gusare [FLAC]/08. MILABO.flac"
# ARGS=-u 127.0.0.1 seznam.cz ipv6.txt ../examples/udp_data.txt

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
