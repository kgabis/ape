#!/bin/bash

set -e

dir=`mktemp -d /tmp/ape.XXXXXXX`
echo "Temp dir: ${dir}"

echo "Splitting and copying files"
../utils/split.py --input ../ape.c --output-path ${dir}
cp ../ape.h ${dir}

cp -r *.c *h files/* ${dir}
echo "    OK"

cd ${dir}

function compile_and_run {
    extra_flags=$1
    echo "Compiling tests (${extra_flags})"
    flags="-Wall -Wextra -pedantic-errors -Werror"
    gcc ${flags} ${extra_flags} -DAPE_TESTS_MAIN *.c -o tests -lm
    echo "    OK"

    echo "Running tests (${extra_flags})"
    ./tests
    echo "    OK"
}

compile_and_run ""

compile_and_run "-g -DAPE_DEBUG -DCOLLECTIONS_DEBUG"
