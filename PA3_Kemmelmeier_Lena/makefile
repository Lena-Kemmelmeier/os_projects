threaded_sum: threaded_sum.o
	gcc -Wall -pthread -o threaded_sum threaded_sum.o

threaded_sum.o: threaded_sum.c
	gcc -Wall -pthread  -c threaded_sum.c

clean:
	rm -f *.o threaded_sum