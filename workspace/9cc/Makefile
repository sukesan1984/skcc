//CFLAGS=-Wall -Werror -Wextra -std=c11 -g -static
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TEST_FILE=test/test.c

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc $(TEST_FILE)
	@./9cc -Iinclude  -I/usr/local/include -I/usr/include \
		-I/usr/include/linux -I/usr/include/x86_64-linux-gnu \
		-I/usr/lib/gcc/x86_64-linux-gnu/7.5.0/include \
		-I.  $(TEST_FILE) > tmp-test.s
	echo 'int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	@gcc -static -o tmp-test tmp-test.s test/external.c tmp2.o
	@./tmp-test
	@./9cc -I. test/comment.c > tmp-test2.s
	@gcc -static -o tmp-test2 tmp-test2.s
	@./tmp-test2

dump: 9cc
	./dump.sh $(TEST_FILE)

sample: 9cc
	@./9cc -Iinclude  -I/usr/local/include -I/usr/include \
		-I/usr/include/linux -I/usr/include/x86_64-linux-gnu \
		-I/usr/lib/gcc/x86_64-linux-gnu/7.5.0/include \
		-I. test/sample.c > tmp-sample.s
	#gcc -static -o tmp tmp-sample.s test/external.c
	gcc -o tmp tmp-sample.s test/external.c
	cat tmp-sample.s
	@./tmp

break: 9cc
	gdb 9cc
clean:
	rm -f 9cc *.o *~ tmp* a.out test/*~
.PHONY: test clean
