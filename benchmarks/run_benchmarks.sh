#!/bin/bash

set -e

dir=`mktemp -d /tmp/ape.XXXXXXX`
echo "Temp dir: ${dir}"

echo "Copying files"
cp ../ape.h ../ape.c ${dir}
cp *.c files/*.ape ${dir}
echo "    OK"

cd ${dir}
echo "Compiling benchmarks"
gcc -O3 -DAPE_BENCHMARKS_MAIN *.c -o benchmarks -lm
echo "    OK"

echo "Running benchmarks"
./benchmarks strings.ape
./benchmarks mergesort.ape
./benchmarks raytracer_profile.ape
./benchmarks raytracer_profile_optimised.ape
./benchmarks primes.ape
./benchmarks fibonacci.ape
echo "    OK"
