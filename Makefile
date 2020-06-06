build:
	docker build -t sukesan1984/make_compiler .

test:
	make build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make test'

self:
	make build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make self'

self-test:
	make build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make self-test'

dump: build
	docker run --rm sukesan1984/make_compiler bash -c 'cd 9cc; make clean && make dump'

sample: build
	docker run --rm sukesan1984/make_compiler bash -c 'cd 9cc; make clean && make sample'

in: build
	docker run --rm -i -t --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" sukesan1984/make_compiler bash

.PHONY: in test build
