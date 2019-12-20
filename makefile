main: start.o reader_api.o
	gcc $(CFLAGS) -o start start.o reader_api.o

start.o: start.c reader_api.h
	gcc $(CFLAGS) -c start.c

reader_api.o: reader_api.c reader_api.h
	gcc $(CFLAGS) -c reader_api.c



clean:
	rm -f *.o reciprocal
