import sys
import os.path
import os
import subprocess
from subprocess import Popen, PIPE, STDOUT

t_no = 1
i_no = 0

err_msg = "in dump example/jsondump.c:44"

my_env = os.environ.copy()
my_env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"

sys.setrecursionlimit(10**6)

def is_fail_path(jsondump_path, in_path):

    fp = open(in_path, "rb")
    in_byte = fp.read()
    fp.close()

    p = Popen([jsondump_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, env=my_env)
    p.stdin.write(in_byte)
    out = p.communicate()[0].decode("utf-8")
    #print(out)

    return err_msg in out

def is_fail(jsondump_path, in_str):

    p = Popen([jsondump_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, env=my_env)
    p.stdin.write(in_str.encode("utf-8"))
    out = p.communicate()[0].decode("utf-8")

    return err_msg in out


def test():
    fail = is_fail("./jsondump", "./crash.json")
    if fail:
        print("PASS!")
    else:
        print("FAIL!")
    
    fail = is_fail("./jsondump", "./library.json")
    if not fail:
        print("PASS!")
    else:
        print("FAIL!")


def mymin(jsondum_path, input_path):
    global t_no
    fp = open(input_path, "r")
    T = fp.read()
    fp.close()

    if len(T) < 3:
        return input_path

    for range_size in range(len(T)-1-1, -1, -1):
        partitions = split(T, range_size)
        for i, p in enumerate(partitions):
            t_no = t_no + 1
            if is_fail(jsondump_path, p):
                name = "./"+str(range_size)+ "_"+ str(i) + ".part"
                fp = open(name, "w")
                fp.write(p)
                fp.close()
                return mymin(jsondum_path, name)

    return input_path


def split(T, range_size):
    partitions = []
    for k in range(0, len(T)-range_size):
        #_range = T[k:k+range_size]
        partitions.append(T[:k]+T[k+range_size+1:])

    return partitions


#test()

jsondump_path = sys.argv[1]
init_input_path = sys.argv[2]

r_path = mymin(jsondump_path, init_input_path)
print("path:", r_path, ", test number:", t_no)
