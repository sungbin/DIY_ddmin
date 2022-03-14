git clone https://github.com/zserge/jsmn.git
cd jsmn
git checkout 6784c826d9674915a4d89649c6288e6aecb4110d
export ASAN_OPTIONS=detect_leaks=0
CFLAGS="-O3 -g -fsanitize=address" LDFLAGS="-fsanitize=address" make jsondump
mv jsondump ..
cd ..
