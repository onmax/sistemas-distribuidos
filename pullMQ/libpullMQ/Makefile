CFLAGS=-Wall -g -fPIC

libpullMQ.so: libpullMQ.o comun.o
	$(CC) $(CFLAGS) -shared -o $@ $^

libpullMQ.o: comun.h  pullMQ.h

clean:
	rm -f *.o *.so


