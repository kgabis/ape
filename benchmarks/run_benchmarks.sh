#!/bin/bash

set -e

dir=`mktemp -d /tmp/foo.XXXXXXX`
echo "Temp dir: ${dir}"

echo "Copying files"
cp ../ape.h ../ape.c ${dir}

cp *.c files/*.bn ${dir}
echo "    OK"

cd ${dir}
echo "Compiling benchmarks"
gcc -O3 -DAPE_BENCHMARKS_MAIN *.c -o benchmarks
echo "    OK"

echo "Running benchmarks"
./benchmarks mergesort.bn
./benchmarks raytracer_profile.bn
./benchmarks raytracer_profile_optimised.bn
./benchmarks primes.bn
./benchmarks fibonacci.bn
echo "    OK"
