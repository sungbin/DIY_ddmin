import sys
import os.path
import os
import subprocess
from subprocess import Popen, PIPE, STDOUT, run

my_env = os.environ.copy()
my_env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"

sys.setrecursionlimit(10**6)
os.system("rm *.part")

def mymin(file_name):
    fp = open(file_name, "r")
    mtr = fp.read()
    fp.close()

    print("minimized: ", file_name, len(mtr))

   
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

    print("*\t*\t*", r_size, r_begin)

    #e_partion = t[r_begin:r_end]
    partition = t[0:r_begin] + t[r_end:]

    name = "./"+str(r_size)+ "_"+ str(r_begin) + ".part"

    fp = open(name, "w")
    fp.write(partition)
    fp.close()

    fp = open(name, "r")

    cmod = ["./jsondump", "/dev/null"]
    #cmod = "ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./jsondump < " + name

    #p = run(cmod, stderr=PIPE, input=fp.read(), encoding='ascii')
    p = subprocess.Popen(cmod, stderr=PIPE, stdout=PIPE, stdin=PIPE, env=my_env)
    stdout, stderr = p.communicate(input=fp.read().encode('ascii'))

    #print(p.returncode)
    #print(stderr)

    if '0x615000000280 is located 0 bytes to the right of 512-byte region'.encode('ascii') in stderr:
        fp.close()
        os.rename(name, "last_minimized.txt")

        return mymin("./last_minimized.txt")
    else:
        os.remove( name)
        fp.close()

    _mymin(t, r_size, r_begin+1)


os.system("rm *.part")
#init_input_path = '../9503.part'
init_input_path = './crash.json'
try:
    print(mymin(init_input_path))
except:
    print("catched!")
    sys.exit(1)

