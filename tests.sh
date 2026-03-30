#!/bin/bash

# hey man, i never said i would handcode the test cases :P

PASS=0
FAIL=0

check() {
    local input="$1"
    local expected="$2"

    echo "$input" | ./a.out > /dev/null 2>&1
    ./jcc.out
    local actual=$?

    if [ "$actual" -eq "$expected" ]; then
        echo "| PASS: $input -> $expected"
        PASS=$((PASS + 1))
    else
        echo "X FAIL: $input -> expected $expected got $actual"
        FAIL=$((FAIL + 1))
    fi
}

gcc jcc.c

# arithmetic
check "5 + 3"       8
check "10 - 4"      6
check "3 * 7"       21
check "20 / 4"      5
check "10 % 3"      1
check "2 + 3 * 4"   14

# bitwise
check "1 << 4"      16
check "32 >> 2"     8
check "12 & 10"     8
check "12 | 3"      15
check "12 ^ 10"     6

# comparison
check "3 < 5"   1
check "5 < 3"   0
check "4 == 4"  1
check "4 == 5"  0

# short circuit
check "1 && 1"       1
check "1 && 0"       0
check "0 && 1"       0
check "0 && 0"       0
check "1 || 1"       1
check "1 || 0"       1
check "0 || 1"       1
check "0 || 0"       0

echo ""
echo "$PASS passed, $FAIL failed"
