TARGET=../dns_receiver
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

#########################

.PHONY: all run clean valgrind debug

all: $(TARGET)

run: $(TARGET)
	$<

$(TARGET): $(OBJS)
	cd .. && make dns_receiver

clean: 
	rm -f $(TARGET) *.o

valgrind:
	cd .. && make valgrind_r

debug:
	cd .. && make debug_r
