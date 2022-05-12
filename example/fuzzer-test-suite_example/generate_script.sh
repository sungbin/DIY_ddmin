#!/bin/bash

REDUCE_EXE="../reduce"

# LIBXML2
LIBXML2_EXE="../libxml2/libxml2-v2.9.2-fsanitize_fuzzer"
LIBXML2_INPUT_PREFIX="../fuzzer-test-suite/libxml2-v2.9.2/"

LIST_LIBXML2_INPUT=("crash-50b12d37d6968a2cd9eb3665d158d9a2fb1f6e28" "crash-d8960e21ca40ea5dc60ad655000842376d4178a1" "leak-bdbb2857b7a086f003db1c418e1d124181341fb1" "uaf-1153fbf466b9474e6e3c48c72e86a4726b449ef7")

LIST_LIBXML2_INPUT_ERR=("AddressSanitizer: heap-buffer-overflow" "AddressSanitizer: heap-buffer-overflow" "LeakSanitizer: detected memory leaks" "AddressSanitizer: heap-use-after-free")

# SQLITE
SQLITE_EXE="../sqlite/sqlite-2016-11-14-fsanitize_fuzzer"
SQLITE_INPUT_PREFIX="../fuzzer-test-suite/sqlite-2016-11-14/"

LIST_SQLITE_INPUT=("leak-b0276985af5aa23c98d9abf33856ce069ef600e2" "crash-0adc497ccfcc1a4d5e031b735c599df0cae3f4eb" "crash-1066e42866aad3a04e6851dc494ad54bc31b9f78")

LIST_SQLITE_INPUT_ERR=("LeakSanitizer: detected memory leaks" "AddressSanitizer: heap-use-after-free" "AddressSanitizer: heap-use-after-free")

# HARFBUZ
HARFBUZ_EXE="../harfbuzz/harfbuzz-1.3.2-fsanitize_fuzzer"
HARFBUZ_INPUT_PREFIX="../fuzzer-test-suite/harfbuzz-1.3.2/"

LIST_HARFBUZ_INPUT=("crash-ac7c48dc90984a43c1424106d7046c101ada4bf3")
LIST_HARFBUZ_INPUT_ERR=("ERROR: libFuzzer: deadly signal")

# LCMS
LCMS_EXE="../lcms/lcms-2017-03-21-fsanitize_fuzzer"
LCMS_INPUT_PREFIX="../fuzzer-test-suite/lcms-2017-03-21/"

LIST_LCMS_INPUT=("crash-6a7f7b35fc6de5b19080b1c32588c727caf5d396")

LIST_LCMS_INPUT_ERR=("AddressSanitizer: heap-buffer-overflow")

# OPENTHREAD
OPENTHREAD_EXE="../openthread/openthread-2018-02-27-fsanitize_fuzzer-ip6"
OPENTHREAD_INPUT_PREFIX="../fuzzer-test-suite/openthread-2018-02-27/"

LIST_OPENTHREAD_INPUT=("repro5" "repro6")
LIST_OPENTHREAD_INPUT_ERR=("AddressSanitizer: stack-buffer-overflow" "AddressSanitizer: stack-buffer-overflow")

# VORBIS
VORBIS_EXE="../vorbis/vorbis-2017-12-11-fsanitize_fuzzer"
VORBIS_INPUT_PREFIX="../fuzzer-test-suite/vorbis-2017-12-11/"

LIST_VORBIS_INPUT=("crash-23c2d78e497bf4aebe5859e3092657cb0af4c299" "crash-8c5dea6410b0fb0b21ff968a9966a0bd7956405f")

LIST_VORBIS_INPUT_ERR=("AddressSanitizer: SEGV on unknown address" "AddressSanitizer: heap-buffer-overflow")


# LIBXML2 script
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

# SQLITE script
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

# HARFBUZ script
for (( i = 0 ; i < ${#LIST_HARFBUZ_INPUT[@]} ; i++ )) ; do
	HARFBUZ_INPUT="${HARFBUZ_INPUT_PREFIX}${LIST_HARFBUZ_INPUT[$i]}"
	ERR_MSG=${LIST_HARFBUZ_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_HARFBUZ_INPUT[$i]}_ddmin" > "script$((i+7))_ddmin"
	echo "mkdir result_${LIST_HARFBUZ_INPUT[$i]}_ddmin" >> "script$((i+7))_ddmin"
	echo "cd result_${LIST_HARFBUZ_INPUT[$i]}_ddmin" >> "script$((i+7))_ddmin"
	echo "${REDUCE_EXE} ${HARFBUZ_EXE} ${HARFBUZ_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+7))_ddmin"
	echo "cd .." >> "script$((i+7))_ddmin"
	echo "echo \"done ddmin on $HARFBUZ_INPUT\"" >> "script$((i+7))_ddmin"
	echo "" >> "script$((i+7))_ddmin"
	chmod 755 "script$((i+7))_ddmin"

	echo "rm -r result_${LIST_HARFBUZ_INPUT[$i]}_range" > "script$((i+7))_range"
	echo "mkdir result_${LIST_HARFBUZ_INPUT[$i]}_range" >> "script$((i+7))_range"
	echo "cd result_${LIST_HARFBUZ_INPUT[$i]}_range" >> "script$((i+7))_range"
	echo "${REDUCE_EXE} ${HARFBUZ_EXE} ${HARFBUZ_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+7))_range"
	echo "cd .." >> "script$((i+7))_range"
	echo "echo \"done range on $HARFBUZ_INPUT\"" >> "script$((i+7))_range"
	echo "" >> "script$((i+7))_range"
	chmod 755 "script$((i+7))_range"

	echo "rm -r result_${LIST_HARFBUZ_INPUT[$i]}_range_inc" > "script$((i+7))_range_inc"
	echo "mkdir result_${LIST_HARFBUZ_INPUT[$i]}_range_inc" >> "script$((i+7))_range_inc"
	echo "cd result_${LIST_HARFBUZ_INPUT[$i]}_range_inc" >> "script$((i+7))_range_inc"
	echo "${REDUCE_EXE} ${HARFBUZ_EXE} ${HARFBUZ_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+7))_range_inc"
	echo "cd .." >> "script$((i+7))_range_inc"
	echo "echo \"done range-inc on $HARFBUZ_INPUT\"" >> "script$((i+7))_range_inc"
	echo "" >> "script$((i+7))_range_inc"
	chmod 755 "script$((i+7))_range_inc"

done

# LCMS script
for (( i = 0 ; i < ${#LIST_LCMS_INPUT[@]} ; i++ )) ; do
	LCMS_INPUT="${LCMS_INPUT_PREFIX}${LIST_LCMS_INPUT[$i]}"
	ERR_MSG=${LIST_LCMS_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_LCMS_INPUT[$i]}_ddmin" > "script$((i+8))_ddmin"
	echo "mkdir result_${LIST_LCMS_INPUT[$i]}_ddmin" >> "script$((i+8))_ddmin"
	echo "cd result_${LIST_LCMS_INPUT[$i]}_ddmin" >> "script$((i+8))_ddmin"
	echo "${REDUCE_EXE} ${LCMS_EXE} ${LCMS_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+8))_ddmin"
	echo "cd .." >> "script$((i+8))_ddmin"
	echo "echo \"done ddmin on $LCMS_INPUT\"" >> "script$((i+8))_ddmin"
	echo "" >> "script$((i+8))_ddmin"
	chmod 755 "script$((i+8))_ddmin"

	echo "rm -r result_${LIST_LCMS_INPUT[$i]}_range" > "script$((i+8))_range"
	echo "mkdir result_${LIST_LCMS_INPUT[$i]}_range" >> "script$((i+8))_range"
	echo "cd result_${LIST_LCMS_INPUT[$i]}_range" >> "script$((i+8))_range"
	echo "${REDUCE_EXE} ${LCMS_EXE} ${LCMS_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+8))_range"
	echo "cd .." >> "script$((i+8))_range"
	echo "echo \"done range on $LCMS_INPUT\"" >> "script$((i+8))_range"
	echo "" >> "script$((i+8))_range"
	chmod 755 "script$((i+8))_range"

	echo "rm -r result_${LIST_LCMS_INPUT[$i]}_range_inc" > "script$((i+8))_range_inc"
	echo "mkdir result_${LIST_LCMS_INPUT[$i]}_range_inc" >> "script$((i+8))_range_inc"
	echo "cd result_${LIST_LCMS_INPUT[$i]}_range_inc" >> "script$((i+8))_range_inc"
	echo "${REDUCE_EXE} ${LCMS_EXE} ${LCMS_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+8))_range_inc"
	echo "cd .." >> "script$((i+8))_range_inc"
	echo "echo \"done range-inc on $LCMS_INPUT\"" >> "script$((i+8))_range_inc"
	echo "" >> "script$((i+8))_range_inc"
	chmod 755 "script$((i+8))_range_inc"

done

# OPENTHREAD script
for (( i = 0 ; i < ${#LIST_OPENTHREAD_INPUT[@]} ; i++ )) ; do
	OPENTHREAD_INPUT="${OPENTHREAD_INPUT_PREFIX}${LIST_OPENTHREAD_INPUT[$i]}"
	ERR_MSG=${LIST_OPENTHREAD_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_OPENTHREAD_INPUT[$i]}_ddmin" > "script$((i+9))_ddmin"
	echo "mkdir result_${LIST_OPENTHREAD_INPUT[$i]}_ddmin" >> "script$((i+9))_ddmin"
	echo "cd result_${LIST_OPENTHREAD_INPUT[$i]}_ddmin" >> "script$((i+9))_ddmin"
	echo "${REDUCE_EXE} ${OPENTHREAD_EXE} ${OPENTHREAD_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+9))_ddmin"
	echo "cd .." >> "script$((i+9))_ddmin"
	echo "echo \"done ddmin on $OPENTHREAD_INPUT\"" >> "script$((i+9))_ddmin"
	echo "" >> "script$((i+9))_ddmin"
	chmod 755 "script$((i+9))_ddmin"

	echo "rm -r result_${LIST_OPENTHREAD_INPUT[$i]}_range" > "script$((i+9))_range"
	echo "mkdir result_${LIST_OPENTHREAD_INPUT[$i]}_range" >> "script$((i+9))_range"
	echo "cd result_${LIST_OPENTHREAD_INPUT[$i]}_range" >> "script$((i+9))_range"
	echo "${REDUCE_EXE} ${OPENTHREAD_EXE} ${OPENTHREAD_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+9))_range"
	echo "cd .." >> "script$((i+9))_range"
	echo "echo \"done range on $OPENTHREAD_INPUT\"" >> "script$((i+9))_range"
	echo "" >> "script$((i+9))_range"
	chmod 755 "script$((i+9))_range"

	echo "rm -r result_${LIST_OPENTHREAD_INPUT[$i]}_range_inc" > "script$((i+9))_range_inc"
	echo "mkdir result_${LIST_OPENTHREAD_INPUT[$i]}_range_inc" >> "script$((i+9))_range_inc"
	echo "cd result_${LIST_OPENTHREAD_INPUT[$i]}_range_inc" >> "script$((i+9))_range_inc"
	echo "${REDUCE_EXE} ${OPENTHREAD_EXE} ${OPENTHREAD_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+9))_range_inc"
	echo "cd .." >> "script$((i+9))_range_inc"
	echo "echo \"done range-inc on $OPENTHREAD_INPUT\"" >> "script$((i+9))_range_inc"
	echo "" >> "script$((i+9))_range_inc"
	chmod 755 "script$((i+9))_range_inc"

done

# VORBIS script
for (( i = 0 ; i < ${#LIST_VORBIS_INPUT[@]} ; i++ )) ; do
	VORBIS_INPUT="${VORBIS_INPUT_PREFIX}${LIST_VORBIS_INPUT[$i]}"
	ERR_MSG=${LIST_VORBIS_INPUT_ERR[$i]}
	echo "rm -r result_${LIST_VORBIS_INPUT[$i]}_ddmin" > "script$((i+11))_ddmin"
	echo "mkdir result_${LIST_VORBIS_INPUT[$i]}_ddmin" >> "script$((i+11))_ddmin"
	echo "cd result_${LIST_VORBIS_INPUT[$i]}_ddmin" >> "script$((i+11))_ddmin"
	echo "${REDUCE_EXE} ${VORBIS_EXE} ${VORBIS_INPUT} ddmin \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+11))_ddmin"
	echo "cd .." >> "script$((i+11))_ddmin"
	echo "echo \"done ddmin on $VORBIS_INPUT\"" >> "script$((i+11))_ddmin"
	echo "" >> "script$((i+11))_ddmin"
	chmod 755 "script$((i+11))_ddmin"

	echo "rm -r result_${LIST_VORBIS_INPUT[$i]}_range" > "script$((i+11))_range"
	echo "mkdir result_${LIST_VORBIS_INPUT[$i]}_range" >> "script$((i+11))_range"
	echo "cd result_${LIST_VORBIS_INPUT[$i]}_range" >> "script$((i+11))_range"
	echo "${REDUCE_EXE} ${VORBIS_EXE} ${VORBIS_INPUT} range \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+11))_range"
	echo "cd .." >> "script$((i+11))_range"
	echo "echo \"done range on $VORBIS_INPUT\"" >> "script$((i+11))_range"
	echo "" >> "script$((i+11))_range"
	chmod 755 "script$((i+11))_range"

	echo "rm -r result_${LIST_VORBIS_INPUT[$i]}_range_inc" > "script$((i+11))_range_inc"
	echo "mkdir result_${LIST_VORBIS_INPUT[$i]}_range_inc" >> "script$((i+11))_range_inc"
	echo "cd result_${LIST_VORBIS_INPUT[$i]}_range_inc" >> "script$((i+11))_range_inc"
	echo "${REDUCE_EXE} ${VORBIS_EXE} ${VORBIS_INPUT} range_increasing \"${ERR_MSG}\"  2>&1 | tee log" >> "script$((i+11))_range_inc"
	echo "cd .." >> "script$((i+11))_range_inc"
	echo "echo \"done range-inc on $VORBIS_INPUT\"" >> "script$((i+11))_range_inc"
	echo "" >> "script$((i+11))_range_inc"
	chmod 755 "script$((i+11))_range_inc"

done


