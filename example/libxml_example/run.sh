#ddmin_libxml2_243.c 

if [ "$1" == "243" ] ; then
	rm *.part
	rm ./ddmin_243
	gcc ../../src/main.c ../../src/ddmin.c ../../src/range.c ./source/runner_libxml2_243.c -g -o ./ddmin_243 -lm

	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_243 "$1_libxml2/xmllint" ./poc ddmin "AddressSanitizer: SEGV on unknown address" 2>&1 | tee ret_ddmin_243
	rm *.part

	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_243 "$1_libxml2/xmllint" ./poc range "AddressSanitizer: SEGV on unknown address" 2>&1 | tee ret_range_243
	rm *.part

elif [ "$1" == "350" ] ; then
	rm *.part
	rm ./ddmin_350
	gcc ../../src/main.c ../../src/ddmin.c ../../src/range.c ./source/runner_libxml2_350.c -g -o ./ddmin_350 -lm

	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_350 "$1_libxml2/xmllint" ./test2.xml ddmin "AddressSanitizer: SEGV on unknown address" 2>&1 | tee ret_ddmin_350

	rm *.part
	ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./ddmin_350 "$1_libxml2/xmllint" ./test2.xml range "AddressSanitizer: SEGV on unknown address" 2>&1 | tee ret_range_350
	rm *.part


else
	echo "put issue id"

fi

