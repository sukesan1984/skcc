build:
	docker build -t sukesan1984/make_compiler .

all: build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make all'

test: build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make test'

self: build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make self'

self-test: build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make self-test'

self2-test: build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make self2-test'

dump: build
	docker run --rm sukesan1984/make_compiler bash -c 'cd 9cc; make clean && make dump'

sample: build
	docker run --rm sukesan1984/make_compiler bash -c 'cd 9cc; make clean && make sample'

in: build
	docker run --rm -i -t --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" sukesan1984/make_compiler bash

.PHONY: in test build
