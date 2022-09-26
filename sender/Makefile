TARGET=../dns_sender
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	$<

$(TARGET): $(OBJS)
	cd .. && make dns_sender

clean:
	rm -f $(TARGET) *.o

valgrind:
	cd .. && make valgrind_s

debug:
	cd .. && make debug_s
