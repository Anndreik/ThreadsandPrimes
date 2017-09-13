HEADERS =

default: threadsandprimes

threadsandprimes.o: threadsandprimes.c $(HEADERS)
	gcc -c -D_REENTRANT threadsandprimes.c -lpthread -o threadsandprimes.o -lm

threadsandprimes: threadsandprimes.o
	gcc -D_REENTRANT threadsandprimes.o -lpthread -o threadsandprimes -lm

clean:
	-rm -f threadsandprimes.o
	-rm -f threadsandprimes
