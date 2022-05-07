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
