git clone https://github.com/php/php-src.git
cd php-src
git checkout 70bd46cdacd9ffecd16c4672b6e42b8c0943cc48

./buildconf
CC=clang CXX=clang++ \
./configure \
    --disable-all \
    --enable-fuzzer \
    --with-pic \
    --enable-debug \
    --enable-debug-assertions \
    --enable-address-sanitizer \
    --enable-exif \
    --enable-mbstring \
    --enable-opcache

make
