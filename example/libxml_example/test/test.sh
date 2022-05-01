#ddmin_libxml2_243.c 

rm *.part
rm ./ddmin_test
gcc ./test.c ../source/ddmin_libxml2_243.c ../source/runner_libxml2_243.c -g -o ./ddmin_test -lm

ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_test "$1_libxml2/xmllint" 2>&1 | tee ret

