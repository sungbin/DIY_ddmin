rm ./bin/*
gcc main.c ../../src/ddmin.c ../../src/runner.c -o ./bin/main -lm -lpthread

ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./bin/main ../jsondump_example/jsondump ../jsondump_example/crash.json "dump example/jsondump.c:44"
