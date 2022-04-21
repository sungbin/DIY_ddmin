# 1. git pull jsmn, and make
#./make_jsondump.sh

# 2. test main

rm ret.txt
rm ./*.part
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ../bin/main ./jsondump ./crash.json mymin >> ret.txt 2>> ret.txt
