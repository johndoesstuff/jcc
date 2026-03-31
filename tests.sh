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

# math
check "5 + 3"       8
check "10 - 4"      6
check "3 * 7"       21
check "20 / 4"      5
check "10 % 3"      1

check "2 + 3 * 4 - 1"          13
check "10 - 2 * 3 + 1"         5
check "6 / 2 + 4 * 3"          15
check "100 / 10 / 2"           5
check "(2 + 3) * 4"            20
check "10 / (2 + 3)"           2
check "(10 - 4) * (2 + 3)"     30
check "7 % 7"                  0
check "1 % 7"                  1
check "15 % 4"                 3

# bitwise
check "1 << 4"      16
check "32 >> 2"     8
check "12 & 10"     8
check "12 | 3"      15
check "12 ^ 10"     6
check "1 << 0"      1
check "0 << 10"     0
check "128 >> 7"    1
check "1 << 3 << 1" 16

# comparison
check "3 < 5"   1
check "5 < 3"   0
check "4 == 4"  1
check "4 == 5"  0

# logic
check "1 && 1"       1
check "1 && 0"       0
check "0 && 1"       0
check "0 && 0"       0
check "1 || 1"       1
check "1 || 0"       1
check "0 || 1"       1
check "0 || 0"       0

check "1 < 2 && 3 < 4"         1
check "1 < 2 && 5 < 4"         0
check "1 > 2 || 3 < 4"         1
check "1 > 2 || 5 < 4"         0
check "1 == 1 && 2 == 2"       1
check "1 == 1 && 2 == 3"       0

# conditional
check "1 ? 5 : 7"    5
check "0 ? 5 : 7"    7
check "4 ? 5 : 7"    5

check "1 ? 2 ? 8 : 9 : 10"    8
check "1 ? 0 ? 8 : 9 : 10"    9
check "0 ? 8 : 1 ? 9 : 10"    9
check "0 ? 8 : 0 ? 9 : 10"    10
check "1 ? 2 + 3 : 10 - 1"    5
check "0 ? 2 + 3 : 10 - 1"    9
check "2 * 3 == 6 ? 1 : 0"    1

echo "|"
echo "> $PASS passed, $FAIL failed"
