#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -static -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input expeted, but got $actual"
        exit 1
    fi
}

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
echo OK
