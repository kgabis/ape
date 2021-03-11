#!/bin/bash

set -e

dir=`mktemp -d /tmp/foo.XXXXXXX`
echo "Temp dir: ${dir}"

echo "Splitting and copying files"
../utils/split.py --input ../ape.c --output-path ${dir}
cp ../ape.h ${dir}

cp -r *.c *h files/* ${dir}
echo "    OK"

cd ${dir}
echo "Compiling tests"
flags="-Wall -Wextra -pedantic-errors -Werror"
gcc ${flags} -DAPE_TESTS_MAIN *.c -o tests -lm
echo "    OK"

echo "Running tests"
./tests
echo "    OK"
