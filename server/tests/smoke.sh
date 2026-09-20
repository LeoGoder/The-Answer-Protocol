#!/bin/bash

HOST="localhost"
PORT="4242"

PASS=0
FAIL=0
TEST_ID=0

run_test() {
    TEST_ID=$((TEST_ID + 1))

    NAME="$1"
    COMMAND="$2"
    EXPECTED="$3"
    USERNAME="smoke_$TEST_ID"

    echo "===== TEST $TEST_ID: $NAME ====="
    echo "> $COMMAND"

    OUTPUT=$(
        printf "CONNECT %s\n%s\nQUIT\n" "$USERNAME" "$COMMAND" |
        timeout 2 nc "$HOST" "$PORT"
    )

    if echo "$OUTPUT" | grep -Fq "$EXPECTED"; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        echo "Expected: $EXPECTED"
        echo "Received:"
        echo "$OUTPUT"
        FAIL=$((FAIL + 1))
    fi

    echo
}


# ========================================
# CONNECTION
# ========================================

run_test "CONNECT" \
    "WHO" \
    "OK connected"


# ========================================
# CORE COMMANDS
# ========================================

run_test "LOOK" \
    "LOOK" \
    '"room":{"id":"loc.square"'

run_test "MOVE" \
    "MOVE north" \
    "OK room=loc.square"

run_test "WHO" \
    "WHO" \
    "OK players=1"


# ========================================
# COMMUNICATION
# ========================================

run_test "CHAT" \
    "CHAT GLOBAL hello" \
    "OK"


# ========================================
# GROUP
# ========================================

run_test "GROUP CREATE" \
    "GROUP CREATE" \
    "OK group=group.demo"

run_test "GROUP INVITE" \
    "GROUP INVITE bob" \
    "OK"

run_test "GROUP JOIN" \
    "GROUP JOIN alice" \
    "OK group=group.demo"

run_test "GROUP LEAVE" \
    "GROUP LEAVE" \
    "OK"


# ========================================
# ITEMS
# ========================================

run_test "TAKE" \
    "TAKE Herbs" \
    "OK taken=item.herbs"

run_test "DROP" \
    "DROP item.herbs" \
    "OK dropped=item.herbs"

run_test "INVENTORY" \
    "INVENTORY" \
    'OK ["item.herbs"]'


# ========================================
# NPC / COMBAT
# ========================================

run_test "TALK" \
    "TALK Baker" \
    "OK Welcome to my bakery!"

run_test "ATTACK" \
    "ATTACK goblin" \
    '"status":"combat"'

run_test "STATUS" \
    "STATUS" \
    '"status":"healthy"'


# ========================================
# QUESTS
# ========================================

run_test "QUEST" \
    "QUEST merchant" \
    '"quest_id":"fetch_herbs"'

run_test "QUESTS" \
    "QUESTS" \
    '"quest_id":"fetch_herbs"'


# ========================================
# RESULT
# ========================================

echo "=============================="
echo "PASS: $PASS"
echo "FAIL: $FAIL"
echo "=============================="

if [ "$FAIL" -ne 0 ]; then
    exit 1
fi