try() {
    input="$1"
    echo "$input" > tmp.c
    try_file tmp.c
}

try_file() {
    filename="$1"

    ./9cc $filename > tmp.s
    cat tmp.s
}

try "int main() { !0 }"
