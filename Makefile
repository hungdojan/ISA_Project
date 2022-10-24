# C project: DNS Tunnel
TARGET=dns_receiver dns_sender
SUBMIT_FILES=Makefile README.md dokumentace.pdf include/* \
			 sender/Makefile sender/*.{c,h} receiver/*.{c,h} receiver/Makefile \
			 shared/*.c


#####################################

.PHONY: all run

all: sender receiver

#####################################

.PHONY: sender receiver

sender: sender/Makefile
	@$(MAKE) -Csender --no-print-directory

receiver: receiver/Makefile
	@$(MAKE) -Creceiver --no-print-directory

#####################################

.PHONY: test_server clean pack

test_server: pack
	# scp xdohun00.tar.gz fedora_local:/root/isa
	# ssh fedora_local /root/isa/unpack.sh
	scp xdohun00.tar.gz eva:/homes/eva/xd/xdohun00/isa
	ssh eva /homes/eva/xd/xdohun00/isa/unpack.sh

clean:
	@$(MAKE) -Csender clean --no-print-directory
	@$(MAKE) -Creceiver clean --no-print-directory
	rm -f ./**/*.o *.tar.gz

pack:
	rm -f xdohun00.tar.gz
	@tar -czvf xdohun00.tar.gz $(SUBMIT_FILES)
