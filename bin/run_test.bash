#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

mkdir -p "run_tests/$1"

TEST_DIR="$(realpath test/$1)"
RUN_DIR="$(realpath run_tests/$1)"
export SWARMC="$(realpath ./swarmc) --test-suite-output"
export TESTSWARM="$(realpath $TEST_DIR/test.swarm)"
export TESTSVI="$(realpath $TEST_DIR/test.svi)"
export TESTJSON="$(realpath $TEST_DIR/test.json)"

"${TEST_DIR}/run" > "$RUN_DIR/out" 2> "$RUN_DIR/err"


diff "$RUN_DIR/out" "$TEST_DIR/out.expected" > "$RUN_DIR/out.diff" 2>&1
OUT_STATUS=$?

diff "$RUN_DIR/err" "$TEST_DIR/err.expected" > "$RUN_DIR/err.diff" 2>&1
ERR_STATUS=$?

if [ $OUT_STATUS -eq 0 ] && [ $ERR_STATUS -eq 0 ]; then
    printf "${GREEN}[PASSING]${NC} ${1}\n"
else
    printf "${RED}[FAILING]${NC} ${1}\n"
    exit 1
fi
