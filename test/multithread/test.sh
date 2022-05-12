rm *.part
rm *.stdout
rm *.stderr
rm multi_range
gcc range.c test_main.c ../../src/ddmin.c ../../src/runner.c -g -o ./multi_range -lpthread -lm

./multi_range ../../example/jsondump_example/jsondump ../../example/jsondump_example/crash.json 2>&1 | tee ret

#./multi_range ../../example/jsondump_example/jsondump ./test_input 2>&1 | tee ret
