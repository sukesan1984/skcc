build:
	docker build -t sukesan1984/make_compiler .

test:
	make build
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make clean && make test'
