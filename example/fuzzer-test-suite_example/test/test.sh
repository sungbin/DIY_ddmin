rm *.part

rm ./reduce_test

gcc ./test.c ../../../src/ddmin.c ../../../src/range.c ../source/runner_f-test-suite.c -g -o ./reduce_test -lm

./reduce_test 2>&1 | tee ret
