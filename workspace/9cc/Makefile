CFLAGS=-Wall -Werror -Wextra -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TEST_FILE=test/test.c

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc $(TEST_FILE)
	@./9cc -Iinclude  -I/usr/local/include -I/usr/include \
		-I/usr/include/linux -I/usr/include/x86_64-linux-gnu \
		-I.  $(TEST_FILE) > tmp-test.s
	@gcc -static -o tmp-test tmp-test.s test/external.c
	@./tmp-test
	@./9cc -I. test/comment.c > tmp-test2.s
	@gcc -static -o tmp-test2 tmp-test2.s
	@./tmp-test2

dump: 9cc
	./dump.sh $(TEST_FILE)

sample: 9cc
	./dump.sh test/sample.c
	gcc -static -o tmp tmp.s test/external.c
	@./tmp

break: 9cc
	gdb 9cc
clean:
	rm -f 9cc *.o *~ tmp* a.out test/*~
.PHONY: test clean
