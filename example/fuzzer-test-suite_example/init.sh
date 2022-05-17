#git clone https://github.com/google/fuzzer-test-suite.git
#mkdir libxml2
#cd libxml2
#../fuzzer-test-suite/libxml2-v2.9.2/build.sh
#cd ..

#mkdir sqlite
#cd sqlite
#../fuzzer-test-suite/sqlite-2016-11-14/build.sh
#cd ..

#mkdir harfbuzz
#cd harfbuzz
#../fuzzer-test-suite/harfbuzz-1.3.2/build.sh
#cd ..

#mkdir lcms
#cd lcms
#../fuzzer-test-suite/lcms-2017-03-21/build.sh
#cd ..

#mkdir openthread
#cd openthread
#REVISION=ab4073980f120bbd4eb9f6d58950f2f03f88dac3 ${FTS}../fuzzer-test-suite/openthread-2018-02-27/build.sh
#cd ..

#mkdir vorbis
#cd vorbis
#../fuzzer-test-suite/vorbis-2017-12-11/build.sh
#..

rm ./reduce
cd source
#gcc main_f-test-suite.c ddmin_f-test-suite.c range_f-test-suite.c runner_f-test-suite.c -g -o ../reduce -lm
gcc ../../../src/main.c ../../../src/ddmin.c ../../../src/range.c runner_f-test-suite.c -g -o ../reduce -lm
cd ..
