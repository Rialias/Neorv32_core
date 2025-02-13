# Include the nanopb provided Makefile rules
include /home/ralias/nanopb/extra/nanopb.mk

# Compiler flags to enable all warnings & debug info
CFLAGS = -ansi -Wall -Werror -g -O0
CFLAGS += -I$(NANOPB_DIR)

all: client

.SUFFIXES:

clean:
	rm -f client fileproto.pb.c fileproto.pb.h

%: %.c fileproto.pb.c
	$(CC) $(CFLAGS) -o $@ $^ $(NANOPB_CORE)
