cd jsmn

make clean
export ASAN_OPTIONS=halt_on_error=1
CFLAGS="-O3 -g -fsanitize=address" LDFLAGS="-fsanitize=address" make jsondump

rm ../jsondump
cp ./jsondump ..
cd ..
