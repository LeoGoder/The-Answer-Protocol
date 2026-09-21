#!/bin/bash

HOST="localhost"
PORT="4242"
PASS=0
FAIL=0
TEST_ID=0

check() {
    TEST_ID=$((TEST_ID + 1))
    NAME="$1"
    COMMAND="$2"
    EXPECTED="$3"
    USERNAME="smoke_$TEST_ID"

    OUTPUT=$(printf "CONNECT %s\n%s\nQUIT\n" "$USERNAME" "$COMMAND" | timeout 2 nc "$HOST" "$PORT")
    RESPONSE=$(printf "%s\n" "$OUTPUT" | sed -n '3p')

    printf "===== TEST %02d: %s =====\n> %s\n" "$TEST_ID" "$NAME" "$COMMAND"
    if [ "$RESPONSE" = "$EXPECTED" ]; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        echo "Expected: $EXPECTED"
        echo "Received: $RESPONSE"
        FAIL=$((FAIL + 1))
    fi
    echo
}

check_raw() {
    TEST_ID=$((TEST_ID + 1))
    NAME="$1"
    INPUT="$2"
    LINE="$3"
    EXPECTED="$4"

    OUTPUT=$(printf "%b" "$INPUT" | timeout 2 nc "$HOST" "$PORT")
    RESPONSE=$(printf "%s\n" "$OUTPUT" | sed -n "${LINE}p")

    printf "===== TEST %02d: %s =====\n" "$TEST_ID" "$NAME"
    if [ "$RESPONSE" = "$EXPECTED" ]; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        echo "Expected: $EXPECTED"
        echo "Received: $RESPONSE"
        FAIL=$((FAIL + 1))
    fi
    echo
}

# Connection
check_raw "GREETING" "QUIT\n" 1 "OK hello proto=1"
check_raw "CONNECT" "CONNECT test_connect\nQUIT\n" 2 "OK connected"
check_raw "QUIT" "CONNECT test_quit\nQUIT\n" 3 "OK bye"

# Room state
check "LOOK START" "LOOK" 'OK {"room":{"id":"room.start","name":"Start Room","description":"The starting room.","exits":{"up":"room.transition"}},"players":["smoke_4"],"items":["apple","banane"],"npcs":[]}'
check "MOVE UP" "MOVE up" "OK room=room.transition"
check "MOVE DOWN FROM START" "MOVE down" "ERR 301 NO_EXIT"

# Other RFC commands
check "WHO" "WHO" "OK players=1"
check "CHAT GLOBAL" "CHAT GLOBAL hello" "OK"
check "CHAT ROOM" "CHAT ROOM hello" "OK"
check "CHAT GROUP" "CHAT GROUP hello" "OK"
check "GROUP CREATE" "GROUP CREATE" "OK group=group.default"
check "GROUP INVITE" "GROUP INVITE player" "OK"
check "GROUP JOIN" "GROUP JOIN leader" "OK group=group.default"
check "GROUP LEAVE" "GROUP LEAVE" "OK"
check "TAKE" "TAKE item.test" "ERR 404 ITEM_NOT_FOUND"
check "DROP" "DROP item.test" "ERR 404 ITEM_NOT_IN_INVENTORY"
check "INVENTORY" "INVENTORY" "OK []"
check "TALK" "TALK npc.test" "ERR 404 NPC_NOT_FOUND"
check "ATTACK" "ATTACK npc.test" "ERR 404 NPC_NOT_FOUND"
check "STATUS" "STATUS" 'OK {"hp":100,"max_hp":100,"status":"healthy"}'
check "QUEST" "QUEST npc.test" "ERR 404 NPC_NOT_FOUND"
check "QUESTS" "QUESTS" "OK []"

# Invalid input
check "LOOK WITH ARG" "LOOK extra" "ERR 400 BAD_REQUEST"
check "MOVE WITHOUT ARG" "MOVE" "ERR 400 BAD_REQUEST"
check "MOVE TOO MANY ARGS" "MOVE up extra" "ERR 400 BAD_REQUEST"
check "BAD CHAT SCOPE" "CHAT PRIVATE hello" "ERR 400 BAD_REQUEST"
check "GROUP WITHOUT ACTION" "GROUP" "ERR 400 BAD_REQUEST"
check "UNKNOWN COMMAND" "BANANA" "ERR 400 UNKNOWN_COMMAND"
check_raw "NOT AUTHENTICATED" "LOOK\nQUIT\n" 2 "ERR 202 NOT_AUTHENTICATED"

# Stateful navigation: start -> transition -> end -> transition
check_raw "START TO TRANSITION" "CONNECT route_test\nMOVE up\nMOVE up\nMOVE down\nQUIT\n" 3 "OK room=room.transition"
check_raw "TRANSITION TO END" "CONNECT route_test2\nMOVE up\nMOVE up\nMOVE down\nQUIT\n" 4 "OK room=room.end"
check_raw "END BACK TO TRANSITION" "CONNECT route_test3\nMOVE up\nMOVE up\nMOVE down\nQUIT\n" 5 "OK room=room.transition"

printf "================================\nPASS: %d\nFAIL: %d\nTOTAL: %d\n================================\n" "$PASS" "$FAIL" "$((PASS + FAIL))"
[ "$FAIL" -eq 0 ]
