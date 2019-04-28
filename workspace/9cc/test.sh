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
echo OK
