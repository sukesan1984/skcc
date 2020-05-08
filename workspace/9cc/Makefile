CFLAGS=-Wall -Werror -Wextra -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TEST_FILE=test/test.c

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc $(TEST_FILE)
	./9cc -test
	@gcc -E -P $(TEST_FILE) > test/tmp.c
	@./9cc test/tmp.c > tmp-test.s
	@gcc -static -o tmp-test tmp-test.s test/external.c
	@./tmp-test

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
