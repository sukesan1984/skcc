//CFLAGS=-Wall -Werror -Wextra -std=c11 -g -static
CFLAGS=-std=c11 -Wno-builtin-declaration-mismatch -Wno-incompatible-pointer-types -g -static
INCLUDES=-Iinclude  -I/usr/local/include -I/usr/include \
-I/usr/include/linux -I/usr/include/x86_64-linux-gnu \
-I/usr/lib/gcc/x86_64-linux-gnu/7.5.0/include \
-I.
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
SELF_OBJS=$(SRCS:.c=.self.o)
SELF2_OBJS=$(SRCS:.c=.self2.o)
TEST_FILE=test/test.c

all: test self-test self2-test

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc $(TEST_FILE)
	@./9cc $(INCLUDES) $(TEST_FILE) > tmp-test.s
	echo 'int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	@gcc -static -o tmp-test tmp-test.s test/external.c tmp2.o
	@./tmp-test
	@./9cc -I. test/comment.c > tmp-test2.s
	@gcc -static -o tmp-test2 tmp-test2.s
	@./tmp-test2

self: 9cc
	#$(CC) $(CFLAGS) -S main.c -o _self_main.s
	./9cc main.c > _self_main.s
	$(CC) $(CFLAGS) -c _self_main.s -o main.self.o

	#$(CC) $(CFLAGS) -S preprocess.c -o _self_preprocess.s
	./9cc preprocess.c > _self_preprocess.s
	$(CC) $(CFLAGS) -c _self_preprocess.s -o preprocess.self.o
	#$(CC) $(CFLAGS) -S parse.c -o _self_parse.s
	./9cc parse.c > _self_parse.s
	$(CC) $(CFLAGS) -c _self_parse.s -o parse.self.o
	#$(CC) $(CFLAGS) -S tokenize.c -o _self_tokenize.s
	./9cc tokenize.c > _self_tokenize.s
	$(CC) $(CFLAGS) -c _self_tokenize.s -o tokenize.self.o
	#$(CC) $(CFLAGS) -S util.c -o _self_util.s
	./9cc util.c > _self_util.s
	$(CC) $(CFLAGS) -c _self_util.s -o util.self.o
	#$(CC) $(CFLAGS) -S sema.c -o _self_sema.s
	./9cc sema.c > _self_sema.s
	$(CC) $(CFLAGS) -c _self_sema.s -o sema.self.o
	#$(CC) $(CFLAGS) -S codegen.c -o _self_codegen.s
	./9cc codegen.c > _self_codegen.s
	$(CC) $(CFLAGS) -c _self_codegen.s -o codegen.self.o
	$(CC) -o self $(SELF_OBJS) $(CFLAGS)

2nd-self: self
	./self main.c > _self_main2.s
	$(CC) $(CFLAGS) -c _self_main2.s -o main.self2.o

	./self preprocess.c > _self_preprocess2.s
	$(CC) $(CFLAGS) -c _self_preprocess2.s -o preprocess.self2.o

	./self parse.c > _self_parse2.s
	$(CC) $(CFLAGS) -c _self_parse2.s -o parse.self2.o

	./self tokenize.c > _self_tokenize2.s
	$(CC) $(CFLAGS) -c _self_tokenize2.s -o tokenize.self2.o

	./self util.c > _self_util2.s
	$(CC) $(CFLAGS) -c _self_util2.s -o util.self2.o

	./self sema.c > _self_sema2.s
	$(CC) $(CFLAGS) -c _self_sema2.s -o sema.self2.o

	./self codegen.c > _self_codegen2.s
	$(CC) $(CFLAGS) -c _self_codegen2.s -o codegen.self2.o
	$(CC) -o 2nd-self $(SELF2_OBJS) $(CFLAGS)


self-test: self
	@./self $(INCLUDES) $(TEST_FILE) > tmp-test.s
	echo 'int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	@gcc -g -static -o tmp-test tmp-test.s test/external.c tmp2.o
	@./tmp-test
	@./self -I. test/comment.c > tmp-test2.s
	@gcc -static -o tmp-test2 tmp-test2.s
	@./tmp-test2

self2-test: 2nd-self
	diff _self_main2.s _self_main.s
	diff _self_preprocess2.s _self_preprocess.s
	diff _self_parse2.s _self_parse.s
	diff _self_tokenize2.s _self_tokenize.s
	diff _self_util2.s _self_util.s
	diff _self_sema2.s _self_sema.s
	diff _self_codegen2.s _self_codegen.s
	@./2nd-self $(INCLUDES) $(TEST_FILE) > tmp-test.s
	echo 'int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	@gcc -g -static -o tmp-test tmp-test.s test/external.c tmp2.o
	@./tmp-test
	@./2nd-self -I. test/comment.c > tmp-test2.s
	@gcc -static -o tmp-test2 tmp-test2.s
	@./tmp-test2

dump: 9cc
	./dump.sh $(TEST_FILE)

sample: 9cc
	@./9cc $(INCLUDES) test/sample.c > tmp-sample.s
	gcc -g -static -o tmp tmp-sample.s test/external.c
	cat tmp-sample.s
	@./tmp -I. codegen.c

break: 9cc
	gdb 9cc
clean:
	rm -f 9cc *.o *~ tmp* a.out test/*~ *.s
.PHONY: test clean
