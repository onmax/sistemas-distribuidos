CFLAGS=-Wall -g

all: libpullMQ test

libpullMQ:
	$(MAKE) -C ../libpullMQ

test: test.o libpullMQ.so
	$(CC) -o $@ $< ./libpullMQ.so

test.o: pullMQ.h

clean:
	rm -f *.o test


