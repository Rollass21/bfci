PROGRAM = bfci # name of executable
CC = gcc # compiler used: gcc
MAINSRC = main.c # main source file, in most cases main.c
SRCS = bfci.c insset.c inssetf.c presets.c # .c files containing funcs
HEADERS = bfci.h tcolors.h # .h files

TESTFILE = testsrcs/beer.bf

CFLAGS = -O0 -ggdb -Wall -pg #-Werror

# 'make' or 'make all' compiles executable
# 'make clean' - removes files produced mid compiling

.PHONY: all clean cls perftest
#====NO TOUCHIE TOUCHIE BELOW THIS LINE====#

OBJS = ${SRCS:.c=.o} $(MAINSRC:.c=.o)

all: $(PROGRAM)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean cls:
	rm -f  $(PROGRAM) $(OBJS)

perftest:
	./$(PROGRAM) -t $(TESTFILE) | gprof bfci gmon.out > perftest.txt
