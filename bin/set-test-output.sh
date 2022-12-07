#!/bin/bash -e

TEST_DIR="$(./bin/resolve-test.sh $1)"
OUTPUT_DIR="$(./bin/resolve-output.sh $1)"

cp "${OUTPUT_DIR}/out" "${TEST_DIR}/out.expected"
cp "${OUTPUT_DIR}/err" "${TEST_DIR}/err.expected"

