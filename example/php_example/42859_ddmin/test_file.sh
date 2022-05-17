#!/bin/bash
if [ $# -eq 0 ] ; then
    echo "Warning: no input"
    exit 0
fi

rm -r ./php-src/Zend/tests/*
rm -r ./php-src/ext/opcache/tests/jit/*

cp $1 ./php-src/Zend/tests
cp $1 ./php-src/ext/opcache/tests/jit/

rm -r ./php-src/inputs
mkdir ./php-src/inputs
cp $1 ./php-src/inputs

cd ./php-src

./sapi/fuzzer/php-fuzz-tracing-jit ./inputs -runs=1
