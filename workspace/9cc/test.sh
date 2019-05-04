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

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 ' 12 + 34 - 5;'
try 37 "5 * 6 + 7;"
try 48 "5 + 6 * 7 + 1;"
try 7 "5 + 14 / 7;"
try 50 "5 + 6 * 7 + 1 * 3;"
try 47 "5 + 6 * 7;"
try 47 "2 + 3 + 6 * 7;"
try 44 "6 / 3 + 6 * 7;"
try 9 "(1 + 2) * 3;"
try 9 "1 + 2;(1 + 2) * 3;"
try 1 "2 == 2;"
try 0 "2 == 3;"
try 0 "2 != 2;"
try 1 "2 != 3;"
try 1 "3 >= 2;"
try 1 "2 >= 2;"
try 0 "1 >= 2;"
try 0 "3 <= 2;"
try 1 "2 <= 2;"
try 1 "1 <= 2;"
try 1 "3 > 2;"
try 0 "2 > 2;"
try 0 "1 > 2;"
try 0 "3 < 2;"
try 0 "2 < 2;"
try 1 "1 < 2;"
try 5 "a = 3;b = 2; a + b;"
try 25 "d = 3 * 2 - 1;b = 5; d * b;"
try 2 "return 2;"
try 5 "return 5; return 6;"
try 10 "return 1 * 2 * 3 + 4;"
try 25 "abc = 3 * 2 - 1;b = 5; abc * b;"
try 4 "a = 1;b = 2; a= 2;return a * b;"
try 50 "abc = 3 * 2 - 1;b = abc + 5; abc * b;"
try 6 "foo = 1; bar = 2 + 3;return foo + bar;"
try 40 "if (2 == 2) return 40;"
try 0 "if (1 == 2) return 40;"
try 3 "if(a = 3) return 3;"
try 0 "if(a = 0) return 3;"
try 10 'a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(b == e) return 10;'
try 0 'a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(a == e) return 0;'
try 0 'a = 1; b = 2; c = 3; d = 2; e = 2; if(a == d) if(a == e) return 0;'
try 5 'a = 1; a = a + 4;return a;'
try 6 'a = 1; while(a != 5) a = a + 1; return 6;'
try 5 'a = 1; while(a != 5) a = a + 1; return a;'

try 11 'b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return b;'
try 5 'b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return i;'
# block
try 5 'a = 1; while(a < 5) { a = a + 1; b = 2; } return a;'
try 5 'a = 1; while(a < 5) { b = 2; a = a + 1; } return a;'
try 2 'a = 1; while(a < 5) { b = 2; a = a + 1; return a; } return a;'
try 11 'b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return b;'
try 5 'b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return i;'
try 0 "if (1 == 2) { return 40; }"
try 3 "if(a = 3) { return 3; }"
try 0 "if(a = 0) { return 3; }"
try 3 'a = 2; if (a != 1) { b = 2; a = 3; } return a;'
try 3 'a = 1; if (a == 1) { b = 4; a = 3; } return a;'
try 4 'a = 1; if (a == 1) { b = 4; a = 3; } return b;'
# function call
try 20 'return hoge();'
try 30 'a = hoge(); b = 10; return a + b;'
try 25 'fuga = fuga(); hoge = hoge(); return fuga + hoge;'
try 3 'return add1(3);'
try 7 'return add2(3, 4);'
try 12 'return add3(3, 4, 5);'
try 18 'return add4(3, 4, 5, 6);'
try 25 'return add5(3, 4, 5, 6, 7);'
try 33 'return add6(3, 4, 5, 6, 7, 8);'
try 36 'return add2(1, 2) + add6(3, 4, 5, 6, 7, 8);'

echo OK
