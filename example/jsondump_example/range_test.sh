rm *.part
#python e_range_non-compl.py ./jsondump ./crash.json
#python e_range_compl.py ./jsondump ./crash.json
python3 i_range_prepost.py ./jsondump ./crash.json | tee ret
#python3 i_range.py ./jsondump ./crash.json | tee ret

