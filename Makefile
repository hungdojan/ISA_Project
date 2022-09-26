# C project: DNS Tunnel
SENDER=dns_sender
RECEIVER=dns_receiver
TARGET=$(SENDER) $(RECEIVER)

#####################################

.PHONY: all

all: $(TARGET)

#####################################

.PHONY: sender receiver

sender: $(SENDER)
	$<

$(SENDER): sender/Makefile
	make -Csender

receiver: $(RECEIVER)
	$<

$(RECEIVER): receiver/Makefile
	make -Creceiver run

#####################################

.PHONY: test_server clean pack

test_server:
	: # TODO: ssh {remote server} [command]

clean:
	rm -f ./**/*.o $(TARGET) *.tar.gz

pack:
	rm -f xdohun00.tar.gz
	tar -czvf xdohun00.tar.gz Makefile README.md sender/{*.h,*.c} receiver/{*.h,*.c} dokumentace.pdf
