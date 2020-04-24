CC=gcc
CFLAGS=-O0 -std=gnu11 -lm -Wextra -Wall -Werror
CFLAG_SAN=$(CFLAGS) -fsanitize=address -g
DEPS=svc.h
OBJ=svc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

svc: $(OBJ)
	
tester: tester.c $(OBJ)
	$(CC) -o $@ $< $(OBJ) $(CFLAG_SAN)
	
.PHONY: clean

clean:
	rm -f *.o
	rm -f test
