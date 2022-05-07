rm *.part
rm reduce_test

rm ../php-src/reduce_test
gcc ./test.c ../../../src/ddmin.c ../../../src/range.c ../source/runner_php.c -g -o ../php-src/reduce_test -lm

cd ../php-src
./reduce_test 2>&1 | tee ../ret
