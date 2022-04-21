rm -r jsmn
git clone https://github.com/zserge/jsmn.git
cd jsmn
git checkout 6784c826d9674915a4d89649c6288e6aecb4110d

export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
CFLAGS="-O0 -g -fsanitize=address" LDFLAGS="-fsanitize=address" make jsondump

rm ../jsondump
cp ./jsondump ..
mv jsondump ..
cd ..
