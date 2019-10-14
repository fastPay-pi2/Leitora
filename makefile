all: start.o reader_api.o
	gcc $(CFLAGS) `pkg-config --cflags gtk+-3.0` -o start start.o reader_api.o `pkg-config --libs gtk+-3.0`
start.o: start.c reader_api.h 
	gcc $(CFLAGS) `pkg-config --cflags gtk+-3.0` -c start.c  `pkg-config --libs gtk+-3.0`
reader_api.o: reader_api.c reader_api.h
	gcc $(CFLAGS) -c reader_api.c
clean:
	rm -f *.o reciprocal
