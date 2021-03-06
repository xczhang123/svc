CC=gcc
CFLAGS=-O0 -std=gnu11 -lm -Wextra #-Wall -Werror
CFLAG_SAN=$(CFLAGS) -fsanitize=address -g
DEPS=svc.h commit_t_dyn_array.h file_t_dyn_array.h
OBJ=svc.o commit_t_dyn_array.o file_t_dyn_array.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

svc: $(OBJ)
	
tester: tester.c $(OBJ)
	$(CC) -o $@ $< $(OBJ) $(CFLAGS) -ggdb3
	valgrind --leak-check=full --log-file="logfile.out" -v ./tester example2
	# valgrind --tool=massif --time-unit=B ./tester example2

.PHONY: clean

clean:
	rm -f *.o
	rm -f tester
	rm -f logfile.out
