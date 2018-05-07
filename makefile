PROGRAM = bfci
CC = gcc

MAINSRC = bfci.c
SRCS = clearptr.c newNode.c moveRight.c moveLeft.c getbfsrc.c printUsage.c
HEADERS = $(MAINSRC:.c=.h)
OBJS = $(MAINSRC:.c=.o) $(SRCS:.c=.o)

.PHONY all clean

all: $(PROGRAM)

$(OBJS): $(MAINSRC) $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(PROGRAM) $(OBJS)
