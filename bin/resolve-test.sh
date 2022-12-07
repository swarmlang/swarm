#!/bin/bash -e


TEST_TAG="$1"

for file in ./test/*${TEST_TAG}*; do
    echo "$file"
done

