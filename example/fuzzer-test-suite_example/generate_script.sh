#!/bin/bash

REDUCE_EXE="../reduce"

LIBXML2_EXE="../libxml2/libxml2-v2.9.2-fsanitize_fuzzer"
LIBXML2_INPUT_PREFIX="../fuzzer-test-suite/libxml2-v2.9.2/"

LIST_LIBXML2_INPUT=("crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28" "crash-d8960e21ca40ea5dc60ad655000842376d4178a1" "leak-bdbb2857b7a086f003db1c418e1d124181341fb1" "uaf-1153fbf466b9474e6e3c48c72e86a4726b449ef7")

LIST_LIBXML2_INPUT_ERR=("AddressSanitizer: heap-buffer-overflow" "AddressSanitizer: heap-buffer-overflow" "LeakSanitizer: detected memory leaks" "AddressSanitizer: heap-use-after-free")

SQLITE_EXE="../sqlite/sqlite-2016-11-14-fsanitize_fuzzer"
SQLITE_INPUT_PREFIX="../fuzzer-test-suite/sqlite-2016-11-14/"

LIST_SQLITE_INPUT=("leak-b0276985af5aa23c98d9abf33856ce069ef600e2" "crash-0adc497ccfcc1a4d5e031b735c599df0cae3f4eb" "crash-1066e42866aad3a04e6851dc494ad54bc31b9f78")

LIST_SQLITE_INPUT_ERR=("LeakSanitizer: detected memory leaks" "AddressSanitizer: heap-use-after-free" "AddressSanitizer: heap-use-after-free")


#echo "" > ./script.sh
#chmod 755 ./script.sh

#set +x

echo "ulimit -n 80000000"

for (( i = 0 ; i < ${#LIST_LIBXML2_INPUT[@]} ; i++ )) ; do
	LIBXML2_INPUT="${LIBXML2_INPUT_PREFIX}${LIST_LIBXML2_INPUT[$i]}"
	ERR_MSG=${LIST_LIBXML2_INPUT_ERR[$i]}
	echo "rm -r result_ddmin_${LIST_LIBXML2_INPUT[$i]}"
	echo "mkdir result_ddmin_${LIST_LIBXML2_INPUT[$i]}"
	echo "cd result_ddmin_${LIST_LIBXML2_INPUT[$i]}"
	echo "${REDUCE_EXE} ${LIBXML2_EXE} ${LIBXML2_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log"
	echo "cd .."
	echo "echo \"done: $LIBXML2_INPUT\""
	echo ""

	echo "rm -r result_range_${LIST_LIBXML2_INPUT[$i]}"
	echo "mkdir result_range_${LIST_LIBXML2_INPUT[$i]}"
	echo "cd result_range_${LIST_LIBXML2_INPUT[$i]}"
	echo "${REDUCE_EXE} ${LIBXML2_EXE} ${LIBXML2_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log"
	echo "cd .."
	echo "echo \"done: $LIBXML2_INPUT\""
	echo ""

done

for (( i = 0 ; i < ${#LIST_SQLITE_INPUT[@]} ; i++ )) ; do
	SQLITE_INPUT="${SQLITE_INPUT_PREFIX}${LIST_SQLITE_INPUT[$i]}"
	ERR_MSG=${LIST_SQLITE_INPUT_ERR[$i]}
	echo "rm -r result_ddmin_${LIST_SQLITE_INPUT[$i]}"
	echo "mkdir result_ddmin_${LIST_SQLITE_INPUT[$i]}"
	echo "cd result_ddmin_${LIST_SQLITE_INPUT[$i]}"
	echo "${REDUCE_EXE} ${SQLITE_EXE} ${SQLITE_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log"
	echo "cd .."
	echo "echo \"done: $SQLITE_INPUT\""
	echo ""

	echo "rm -r result_range_${LIST_SQLITE_INPUT[$i]}"
	echo "mkdir result_ragne_${LIST_SQLITE_INPUT[$i]}"
	echo "cd result_range_${LIST_SQLITE_INPUT[$i]}"
	echo "${REDUCE_EXE} ${SQLITE_EXE} ${SQLITE_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log"
	echo "cd .."
	echo "echo \"done: $SQLITE_INPUT\""
	echo ""
	
done


