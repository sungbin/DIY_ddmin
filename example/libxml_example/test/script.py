from glob import glob

flist = glob("../*.part")

for f in flist:
    print("ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ../243_libxml2/xmllint --recover --postvalid " + f)
