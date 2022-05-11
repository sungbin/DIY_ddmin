rm *.part
rm reduce_test
rm ../php-src/reduce_test

rm -r ../php-src/Zend/tests/*
rm -r ../php-src/ext/opcache/tests/jit/*

gcc ./test.c ../../../../src/ddmin.c ../../../../src/range.c ../../source/runner_php.c -g -o ../php-src/reduce_test -lm

mkdir inputs
rm inputs/*
cp ./$1 ./inputs

cd ../php-src
./reduce_test 2>&1 | tee ../ddmin/ret
