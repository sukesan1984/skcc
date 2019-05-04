build:
	docker build -t sukesan1984/make_compiler .

test:
	docker run --rm sukesan1984/make_compiler  bash -c 'cd 9cc; make test'
