rm multi_range
gcc range.c test_main.c ../../src/ddmin.c ../../src/runner.c -g -o ./multi_range -lpthread -lm

./multi_range ../../example/jsondump_example/jsondump ../../example/jsondump_example/crash.json
