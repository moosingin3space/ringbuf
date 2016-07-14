CC=gcc
CFLAGS=-Wall -Werror -Wextra -O0 -g

all : producer consumer


producer: producer.c ringbuf.c ringbuf.h
	$(CC) $(CFLAGS) producer.c ringbuf.c -o $@

consumer: consumer.c ringbuf.c ringbuf.h
	$(CC) $(CFLAGS) consumer.c ringbuf.c -o $@
