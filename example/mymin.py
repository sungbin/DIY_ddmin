import sys
import os.path
import os
import subprocess
from subprocess import Popen, PIPE, STDOUT, run

#my_env = os.environ.copy()
#my_env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"

sys.setrecursionlimit(10**6)
os.system("rm *.part")
os.system("rm ret")

def mymin(file_name):
    fp = open(file_name, "r")
    mtr = fp.read()
    fp.close()

#    print("minimized: ", file_name, len(mtr))

    if len(mtr) < 2:
        return file_name
    else:
        return _mymin(mtr, len(mtr)-1, 0)

def _mymin(t, r_size, r_begin):
    r_end = r_begin + r_size
    if r_end > len(t):
        if r_size == 1:
            return
        else:
            _mymin(t, r_size-1, 0)
            return

    #e_partion = t[r_begin:r_end]
    partition = t[0:r_begin] + t[r_end:]

    name = "./"+str(r_size)+ "_"+ str(r_begin) + ".part"

    fp = open(name, "w")
    fp.write(partition)
    fp.close()

    cmod = "ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./jsondump < " + name +" >> ret 2>> ret"
    print("echo \"" +cmod + "\" >> ret")
    print(cmod)

    _mymin(t, r_size, r_begin+1)

#init_input_path = '../9503.part'
#init_input_path = './crash.json'
try:
    init_input_path = sys.argv[1]
except:
    print("argv[1]: json file")
    
mymin(init_input_path)

