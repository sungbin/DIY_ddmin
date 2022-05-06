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

for (( i = 0 ; i < ${#LIST_LIBXML2_INPUT[@]} ; i++ )) ; do
	LIBXML2_INPUT="${LIBXML2_INPUT_PREFIX}${LIST_LIBXML2_INPUT[$i]}"
	ERR_MSG=${LIST_LIBXML2_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_LIBXML2_INPUT[$i]}_ddmin" > "script${i}_ddmin"
	echo "mkdir result_${LIST_LIBXML2_INPUT[$i]}_ddmin" >> "script${i}_ddmin"
	echo "cd result_${LIST_LIBXML2_INPUT[$i]}_ddmin" >> "script${i}_ddmin"
	echo "${REDUCE_EXE} ${LIBXML2_EXE} ${LIBXML2_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script${i}_ddmin"
	echo "cd .." >> "script${i}_ddmin"
	echo "echo \"done ddmin on $LIBXML2_INPUT\"" >> "script${i}_ddmin"
	echo "" >> "script${i}_ddmin"
	chmod 755 "script${i}_ddmin"

	echo "rm -r result_${LIST_LIBXML2_INPUT[$i]}_range" > "script${i}_range"
	echo "mkdir result_${LIST_LIBXML2_INPUT[$i]}_range" >> "script${i}_range"
	echo "cd result_${LIST_LIBXML2_INPUT[$i]}_range" >> "script${i}_range"
	echo "${REDUCE_EXE} ${LIBXML2_EXE} ${LIBXML2_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script${i}_range"
	echo "cd .." >> "script${i}_range"
	echo "echo \"done range on $LIBXML2_INPUT\"" >> "script${i}_range"
	echo "" >> "script${i}_range"
	chmod 755 "script${i}_range"

	echo "rm -r result_${LIST_LIBXML2_INPUT[$i]}_range_inc" > "script${i}_range_inc"
	echo "mkdir result_${LIST_LIBXML2_INPUT[$i]}_range_inc" >> "script${i}_range_inc"
	echo "cd result_${LIST_LIBXML2_INPUT[$i]}_range_inc" >> "script${i}_range_inc"
	echo "${REDUCE_EXE} ${LIBXML2_EXE} ${LIBXML2_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script${i}_range_inc"
	echo "cd .." >> "script${i}_range_inc"
	echo "echo \"done range-inc on $LIBXML2_INPUT\"" >> "script${i}_range_inc"
	echo "" >> "script${i}_range_inc"
	chmod 755 "script${i}_range_inc"

done

for (( i = 0 ; i < ${#LIST_SQLITE_INPUT[@]} ; i++ )) ; do
	SQLITE_INPUT="${SQLITE_INPUT_PREFIX}${LIST_SQLITE_INPUT[$i]}"
	ERR_MSG=${LIST_SQLITE_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_SQLITE_INPUT[$i]}_ddmin" > "script$((i+4))_ddmin"
	echo "mkdir result_${LIST_SQLITE_INPUT[$i]}_ddmin" >> "script$((i+4))_ddmin"
	echo "cd result_${LIST_SQLITE_INPUT[$i]}_ddmin" >> "script$((i+4))_ddmin"
	echo "${REDUCE_EXE} ${SQLITE_EXE} ${SQLITE_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+4))_ddmin"
	echo "cd .." >> "script$((i+4))_ddmin"
	echo "echo \"done ddmin on $SQLITE_INPUT\"" >> "script$((i+4))_ddmin"
	echo "" >> "script$((i+4))_ddmin"
	chmod 755 "script$((i+4))_ddmin"

	echo "rm -r result_${LIST_SQLITE_INPUT[$i]}_range" > "script$((i+4))_range"
	echo "mkdir result_${LIST_SQLITE_INPUT[$i]}_range" >> "script$((i+4))_range"
	echo "cd result_${LIST_SQLITE_INPUT[$i]}_range" >> "script$((i+4))_range"
	echo "${REDUCE_EXE} ${SQLITE_EXE} ${SQLITE_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+4))_range"
	echo "cd .." >> "script$((i+4))_range"
	echo "echo \"done range on $SQLITE_INPUT\"" >> "script$((i+4))_range"
	echo "" >> "script$((i+4))_range"
	chmod 755 "script$((i+4))_range"
	
	echo "rm -r result_${LIST_SQLITE_INPUT[$i]}_range_inc" > "script$((i+4))_range_inc"
	echo "mkdir result_${LIST_SQLITE_INPUT[$i]}_range_inc" >> "script$((i+4))_range_inc"
	echo "cd result_${LIST_SQLITE_INPUT[$i]}_range_inc" >> "script$((i+4))_range_inc"
	echo "${REDUCE_EXE} ${SQLITE_EXE} ${SQLITE_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+4))_range_inc"
	echo "cd .." >> "script$((i+4))_range_inc"
	echo "echo \"done range-inc on $SQLITE_INPUT\"" >> "script$((i+4))_range_inc"
	echo "" >> "script$((i+4))_range_inc"
	chmod 755 "script$((i+4))_range_inc"
done


