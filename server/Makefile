CROSS_COMPILE ?=
GCC ?= $(CROSS_COMPILE)gcc

all: aesdsocket

default: aesdsocket

.PHONY: all

aesdsocket: aesdsocket.c
	$(GCC) aesdsocket.c -o aesdsocket

clean:
	rm aesdsocket
