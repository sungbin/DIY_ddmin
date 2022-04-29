#ddmin_libxml2_243.c 

if [ "$1" == "243" ] ; then
	rm *.part
	rm ./ddmin_243
	gcc ../../src/main.c ./source/ddmin_libxml2_243.c ./source/range_libxml2_243.c ./source/runner_libxml2_243.c -g -o ./ddmin_243 -lm
	#ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_243 "$1_libxml2/xmllint" ./poc ddmin | tee ret
	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_243 "$1_libxml2/xmllint" ./poc range | tee ret

elif [ "$1" == "350" ] ; then
	rm *.part
	rm ./ddmin_350
	gcc ../../src/main.c ./source/ddmin_libxml2_243.c ./source/range_libxml2_243.c ./source/runner_libxml2_243.c -g -o ./ddmin_350 -lm
	#ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_350 "$1_libxml2/xmllint" ./test2.xml ddmin | tee ret
	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_350 "$1_libxml2/xmllint" ./test2.xml range | tee ret


else
	echo "put issue id"

fi

