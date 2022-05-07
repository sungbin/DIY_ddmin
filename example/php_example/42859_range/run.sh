rm *.part
rm reduce_test

rm ../php-src/reduce_test
gcc ./test.c ../../../src/ddmin.c ../../../src/range.c ../source/runner_php.c -g -o ../php-src/reduce_test -lm

rm -r ./inputs
mkdir inputs
cp ./test_input ./inputs/test_input

cd ../php-src
./reduce_test 2>&1 | tee ../42859_range/ret
