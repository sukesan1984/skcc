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

try "int main() { struct { char a; char b; int c; } x; x.a = 3; x.b = 5; x.c = 10; return (x.a + x.b + x.c); }"
