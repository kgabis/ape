#!/bin/bash

set -e

dir=`mktemp -d /tmp/foo.XXXXXXX`
echo "Temp dir: ${dir}"

echo "Splitting and copying files"
../utils/split.py --input ../ape.c --output-path ${dir}
cp ../ape.h ${dir}

cp *.c *h files/*.bn ${dir}
echo "    OK"

cd ${dir}
echo "Compiling tests"
# flags="-Wall -Wextra -pedantic-errors -Werror" # todo: fix warnings
flags=""
gcc ${flags} -DAPE_TESTS_MAIN *.c -o tests
echo "    OK"

echo "Running tests"
./tests
echo "    OK"
