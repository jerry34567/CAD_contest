
CC=gcc
CFLAGS=-O3 -DNDEBUG

.c.o:
	$(CC) $(CFLAGS) -c $<

TARGETS= \
aiger.o \
aigtoaig \

all: $(TARGETS)

aigtoaig: aiger.o aigtoaig.o makefile
	$(CC) $(CFLAGS) -o $@ aigtoaig.o aiger.o
aiger.o: aiger.h aiger.c makefile
aigtoaig.o: aigtoaig.c aiger.h makefile
clean:
	rm -f *.o 
	rm -f $(TARGETS)

.PHONY: all clean
