rm *.part
rm a.out
gcc -g test.c ../../src/ddmin.c ../../src/runner.c -lm
./a.out ../jsondump
