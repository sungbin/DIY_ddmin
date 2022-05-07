cd php-src
../reduce ./sapi/fuzzer/php-fuzz-tracing-jit ../inputs ddmin "AddressSanitizer: heap-use-after-free" 2>&1 | tee ../log
