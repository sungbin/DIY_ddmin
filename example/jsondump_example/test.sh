# 1. git pull jsmn, and make
#./make_jsondump.sh

# 2. test main

cd ../..
make clean
make
cd example/jsondump_example/


rm -r result_ddmin
mkdir result_ddmin
cd result_ddmin
rm ./*.part
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ../../../bin/main ../jsondump ../crash.json ddmin "dump example/jsondump.c:44" | tee ret_ddmin
cd ..

rm -r result_range
mkdir result_range
cd result_range
rm ./*.part
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ../../../bin/main ../jsondump ../crash.json range "dump example/jsondump.c:44" | tee ret_range
cd ..
