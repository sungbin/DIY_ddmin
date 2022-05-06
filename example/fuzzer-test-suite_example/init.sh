git clone https://github.com/google/fuzzer-test-suite.git
mkdir libxml2
cd libxml2
../fuzzer-test-suite/libxml2-v2.9.2/build.sh
cd ..

mkdir sqlite
cd sqlite
../fuzzer-test-suite/sqlite-2016-11-14/build.sh
cd ..

rm ./reduce
cd source
gcc main_f-test-suite.c ddmin_f-test-suite.c range_f-test-suite.c runner_f-test-suite.c -g -o ../reduce -lm
cd ..
