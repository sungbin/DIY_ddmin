rm *.part

rm ./reduce_test
rm ./reduce_test_fplus
rm ./old_reduce_test

gcc ./test.c ../source/ddmin_f-test-suite.c ../source/range_f-test-suite.c ../source/runner_f-test-suite.c -g -o ./reduce_test -lm
gcc ./test.c ../source/old_ddmin.c ../source/range_f-test-suite.c ../source/runner_f-test-suite.c -g -o ./old_reduce_test -lm


./reduce_test 2>&1 | tee ret
#./old_reduce_test 2>&1 | tee old_ret

