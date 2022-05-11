rm *.part
rm reduce_test
rm ../php-src/reduce_test

gcc ./test.c ../../../../src/ddmin.c ../../../../src/range.c ../../source/runner_php.c -g -o ../php-src/reduce_test -lm

mkdir inputs
rm inputs/*
cp ./test_input ./inputs

cd ../php-src
./reduce_test 2>&1 | tee ../ddmin/ret
