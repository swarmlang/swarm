#!/bin/bash -e


TEST_TAG="$1"

for file in ./run_tests/*${TEST_TAG}*; do
    echo "$file"
done

