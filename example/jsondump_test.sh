# 1. git pull jsmn, and make
#./make_jsondump.sh

# 2. test main

cd ..
make clean
make

cd example/

rm ./*.part
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ../bin/main ./jsondump ./crash.json ddmin
