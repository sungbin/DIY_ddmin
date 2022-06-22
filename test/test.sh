rm *.part
rm a.out
gcc -g test.c ../src/ddmin.c ../src/runner.c -lm
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./a.out ../example/jsondump_example/jsondump | tee ret
