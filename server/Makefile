CROSS_COMPILE ?=
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -lpthread -lrt

.PHONY: all
all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket $(LDFLAGS)

.PHONY: clean
clean:
	rm -f aesdsocket *.o
