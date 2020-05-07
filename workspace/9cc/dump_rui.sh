try_file() {
    filename="../9cc/test/test.c"
    cd ../rui314
    make
    ./9cc $filename >tmp.s
    cat tmp.s
}

try_file
