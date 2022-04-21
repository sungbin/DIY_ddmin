import sys
import os.path
import os
import subprocess
from subprocess import Popen, PIPE, STDOUT

err_msg = "in dump example/jsondump.c:44"

my_env = os.environ.copy()
my_env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"

sys.setrecursionlimit(10**6)

def test_jsondump(jsondump_path, in_str):

    p = Popen([jsondump_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, env=my_env)
    p.stdin.write(in_str.encode("utf-8"))
    out = p.communicate()[0].decode("utf-8")

    return err_msg in out


def test():
    fail = test_jsondump("./jsondump", "./crash.json")
    if fail:
        print("PASS!")
    else:
        print("FAIL!")
    
    fail = test_jsondump("./jsondump", "./library.json")
    if not fail:
        print("PASS!")
    else:
        print("FAIL!")


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

    #e_partion = t[r_begin:r_end]
    partition = t[0:r_begin] + t[r_end:]

    if test_jsondump(jsondump_path, partition):
        name = "./"+str(r_size)+ "_"+ str(r_begin) + ".part"
        fp = open(name, "w")
        fp.write(partition)
        fp.close()
        return mymin(name)

    _mymin(t, r_size, r_begin+1)

def test_jsondump_path(jsondump_path, in_path):

    fp = open(in_path, "rb")
    in_byte = fp.read()
    fp.close()

    p = Popen([jsondump_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, env=my_env)
    p.stdin.write(in_byte)
    out = p.communicate()[0].decode("utf-8")
    #print(out)

    return err_msg in out


#test()

jsondump_path = sys.argv[1]
init_input_path = sys.argv[2]

mymin(init_input_path)
