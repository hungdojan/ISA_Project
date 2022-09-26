# C project: DNS Tunnel
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic
# CFLAGS+=-O2 # Release
CFLAGS+=-g  # Debug
LIBS=
SENDER=dns_sender
SENDER_OBJS=$(patsubst %.c,%.o,$(wildcard sender/*.c))
RECEIVER=dns_receiver
RECEIVER_OBJS=$(patsubst %.c,%.o,$(wildcard receiver/*.c))
TARGET=$(SENDER) $(RECEIVER)

#####################################

.PHONY: all

all: $(TARGET)

#####################################

.PHONY: sender valgrind_s debug_s

sender: $(SENDER)
	./$<

$(SENDER): $(SENDER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

valgrind_s: $(SENDER)
	valgrind --leak-check=full --track-origins=yes ./$<

debug_s: $(SENDER)
	gdb -tui ./$<

#####################################

.PHONY: receiver valgrind_r debug_r

receiver: $(RECEIVER)
	./$<

$(RECEIVER): $(RECEIVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

valgrind_r: $(RECEIVER)
	valgrind --leak-check=full --track-origins=yes ./$<

debug_r: $(RECEIVER)
	gdb -tui ./$<

#####################################

.PHONY: test_server clean pack

test_server:
	: # TODO: ssh {remote server} [command]

clean:
	rm -f ./**/*.o $(TARGET) *.tar.gz

pack:
	rm -f xdohun00.tar.gz
	tar -czvf xdohun00.tar.gz Makefile README.md sender/{*.h,*.c} receiver/{*.h,*.c} dokumentace.pdf
