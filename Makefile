CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

all: libquipu.a test_quipu

libquipu.a: src/quipu.o
	ar rcs $@ $^

src/quipu.o: src/quipu.c src/quipu.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_quipu: tests/test_quipu.c libquipu.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lquipu -lm

test: test_quipu
	./test_quipu

clean:
	rm -f src/*.o libquipu.a test_quipu
