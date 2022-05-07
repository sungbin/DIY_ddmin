rm ./inputs/*
rm -r ./php-src/Zend/tests/*
rm -r ./php-src/ext/opcache/tests/jit/*

#cp ./119.part ./inputs/test_input
cp ./test_input ./inputs/test_input
cp ./inputs/* ./php-src/Zend/tests
cp ./inputs/* ./php-src/ext/opcache/tests/jit

cd php-src
./sapi/fuzzer/php-fuzz-tracing-jit ../inputs --runs=1
