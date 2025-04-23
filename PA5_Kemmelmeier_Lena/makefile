myfs: myfs.o
	gcc -Wall -o myfs myfs.o

myfs.o: mymalloc.c
	gcc -Wall -c myfs.c

clean:
	rm -f *.o myfs