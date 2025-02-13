# Include the nanopb provided Makefile rules
include /home/ralias/nanopb/extra/nanopb.mk

# Compiler flags to enable all warnings & debug info
CFLAGS = -ansi -Wall -Werror -g -O0
CFLAGS += -I$(NANOPB_DIR)

all: server client

.SUFFIXES:

clean:
	rm -f server client main.pb.c main.pb.h

%: %.c main.pb.c
	$(CC) $(CFLAGS) -o $@ $^ $(NANOPB_CORE)
