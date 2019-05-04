#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -static -o tmp tmp.s tmp-hoge.o tmp-fuga.o tmp-add1.o tmp-add2.o tmp-add3.o tmp-add4.o tmp-add5.o tmp-add6.o
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input expeted, but got $actual"
        exit 1
    fi
}

echo 'int fuga() { return 5; }' | gcc -xc -c -o tmp-fuga.o -
echo 'int hoge() { return 20; }' | gcc -xc -c -o tmp-hoge.o -
echo 'int add1(int a) { return a; }' | gcc -xc -c -o tmp-add1.o -
echo 'int add2(int a, int b) { return a + b; }' | gcc -xc -c -o tmp-add2.o -
echo 'int add3(int a, int b, int c) { return a + b + c; }' | gcc -xc -c -o tmp-add3.o -
echo 'int add4(int a, int b, int c, int d) { return a + b + c + d; }' | gcc -xc -c -o tmp-add4.o -
echo 'int add5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }' | gcc -xc -c -o tmp-add5.o -
echo 'int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }' | gcc -xc -c -o tmp-add6.o -

try 0 "main() { 0; }"
try 42 "main() { 42; }"
try 21 "main() { 5+20-4; }"
try 41 "main() {  12 + 34 - 5; }"
try 37 "main() { 5 * 6 + 7; }"
try 48 "main() { 5 + 6 * 7 + 1; }"
try 7 "main() { 5 + 14 / 7; }"
try 50 "main() { 5 + 6 * 7 + 1 * 3; }"
try 47 "main() { 5 + 6 * 7; }"
try 47 "main() { 2 + 3 + 6 * 7; }"
try 44 "main() { 6 / 3 + 6 * 7; }"
try 9 "main() { (1 + 2) * 3; }"
try 9 "main() { 1 + 2;(1 + 2) * 3; }"
try 1 "main() { 2 == 2; }"
try 0 "main() { 2 == 3; }"
try 0 "main() { 2 != 2; }"
try 1 "main() { 2 != 3; }"
try 1 "main() { 3 >= 2; }"
try 1 "main() { 2 >= 2; }"
try 0 "main() { 1 >= 2; }"
try 0 "main() { 3 <= 2; }"
try 1 "main() { 2 <= 2; }"
try 1 "main() { 1 <= 2; }"
try 1 "main() { 3 > 2; }"
try 0 "main() { 2 > 2; }"
try 0 "main() { 1 > 2; }"
try 0 "main() { 3 < 2; }"
try 0 "main() { 2 < 2; }"
try 1 "main() { 1 < 2; }"
try 5 "main() {int a; int b; a = 3;b = 2; a + b; }"
try 25 "main() { int d; int b; d = 3 * 2 - 1;b = 5; d * b; }"
try 2 "main() { return 2; }"
try 5 "main() { return 5; return 6; }"
try 10 "main() { return 1 * 2 * 3 + 4; }"
try 25 "main() { int abc; int b; abc = 3 * 2 - 1;b = 5; abc * b; }"
try 4 "main() { int a; int b; a = 1;b = 2; a= 2;return a * b; }"
try 50 "main() { int abc; int b; abc = 3 * 2 - 1;b = abc + 5; abc * b; }"
try 6 "main() { int foo; int bar; foo = 1; bar = 2 + 3;return foo + bar; }"
try 40 "main() { if (2 == 2) return 40; }"
try 0 "main() { if (1 == 2) return 40; }"
try 3 "main() { int a; if(a = 3) return 3; }"
try 0 "main() { int a; if(a = 0) return 3; }"
try 10 "main() { int a; int b; int c; int d; int e;  a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(b == e) return 10; }"
try 0 "main() { int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(a == e) return 0; }"
try 0 "main() { int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(a == d) if(a == e) return 0; }"
try 5 "main() { int a; a = 1; a = a + 4;return a; }"
try 6 "main() { int a; a = 1; while(a != 5) a = a + 1; return 6; }"
try 5 "main() { int a; a = 1; while(a != 5) a = a + 1; return a; }"

try 11 "main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return b; }"
try 5 "main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return i; }"
# block
try 5 "main() { int a; int b; a = 1; while(a < 5) { a = a + 1; b = 2; } return a; }"
try 5 "main() { int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; } return a; }"
try 2 "main() { int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; return a; } return a; }"
try 11 "main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return b; }"
try 5 "main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return i; }"
try 0 "main() { if (1 == 2) { return 40; } }"
try 3 "main() { int a; if(a = 3) { return 3; } }"
try 0 "main() { int a; if(a = 0) { return 3; } }"
try 3 "main() { int a; int b; a = 2; if (a != 1) { b = 2; a = 3; } return a; }"
try 3 "main() { int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return a; }"
try 4 "main() { int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return b; }"
# function call
try 20 "main() { return hoge(); }"
try 30 "main() { int a; int b; a = hoge(); b = 10; return a + b; }"
try 25 "main() { int fuga; int hoge; fuga = fuga(); hoge = hoge(); return fuga + hoge; }"
try 3 "main() { return add1(3); }"
try 7 "main() { return add2(3, 4); }"
try 12 "main() { return add3(3, 4, 5); }"
try 18 "main() { return add4(3, 4, 5, 6); }"
try 25 "main() { return add5(3, 4, 5, 6, 7); }"
try 33 "main() { return add6(3, 4, 5, 6, 7, 8); }"
try 36 "main() { return add2(1, 2) + add6(3, 4, 5, 6, 7, 8); }"
try 1 'one() { return 1; } main() { return one(); }'
try 3 'one() { return 1; } two() { return 2; } main() { return one()+two(); }'
try 7 'add(int x, int y) { return x + y; } main() { return add(3, 4); }'
try 1 'local(int a) { a = 3; return 3; } main() { a = 1; local(a); return a;}'


echo OK
