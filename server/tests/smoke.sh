#!/bin/bash

HOST="localhost"
PORT="4242"

# ========================================
# TEST 01 - GREETING
# ========================================

echo "===== TEST 01: GREETING ====="

OUTPUT=$(printf "CONNECT alice\n" | timeout 2 nc "$HOST" "$PORT")

echo "$OUTPUT"

if echo "$OUTPUT" | grep -q "^OK hello proto=1$"; then
    echo "PASS"
else
    echo "FAIL"
fi


# ========================================
# TEST 02 - CONNECT SEND
# ========================================

echo "===== TEST 02: CONNECT ====="

OUTPUT=$(printf "CONNECT alice\n" | timeout 2 nc "$HOST" "$PORT")

echo "$OUTPUT"

if echo "$OUTPUT" | grep -q "^OK connected$"; then
    echo "PASS"
else
    echo "FAIL"
fi