PROGRAM = bfci # name of executable
CC = gcc # compiler used: gcc
MAINSRC = main.c # main source file, in most cases main.c
SRCS = funcs.c # .c files containing funcs
HEADERS = bfci.h # .h files

CFLAGS = -O0 -ggdb -Wall -Werror

# 'make' or 'make all' compiles executable
# 'make clean' - removes files produced mid compiling

.PHONY: all clean
#====NO TOUCHIE TOUCHIE BELOW THIS LINE====#

OBJS = ${SRCS:.c=.o} $(MAINSRC:.c=.o)

all: $(PROGRAM)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f  $(PROGRAM) $(OBJS)
