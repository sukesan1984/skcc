CFLAGS=-Wall -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./9cc -test
	./test.sh

dump: 9cc
	./dump.sh

break: 9cc
	gdb 9cc
clean:
	rm -f 9cc *.o *~ tmp*
.PHONY: test clean
