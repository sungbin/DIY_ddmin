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


def i_range(jsondum_path, input_path):
    fp = open(input_path, "r")
    T = fp.read()
    fp.close()

    return _i_range(jsondum_path, input_path, len(T)-1)

def _i_range(jsondum_path, input_path, rs):
    global t_no
    fp = open(input_path, "r")
    T = fp.read()
    fp.close()

    print("last_minimized:",input_path, "rs:",rs)
    print("test number:", t_no)

    if len(T) == 1 or rs == 0:
        return input_path

    #print("range: (0," + str(len(T)-rs) + ")")

    for begin in range(len(T) - rs):
        r = T[begin:begin+rs]
        #print("range: T["+str(begin)+":"+str(begin+rs)+"]")
        t_no = t_no+1
        if is_fail(jsondump_path, r):
            name = "./"+str(rs)+ "_"+ str(begin) + ".part"
            fp = open(name, "w")
            fp.write(r)
            fp.close()
            return _i_range(jsondum_path, name, rs-1)

    for begin in range(len(T) - rs):
        r = T[:begin] + T[begin+rs:]
        t_no = t_no+1
        if is_fail(jsondump_path, r):
            name = "./"+str(rs)+ "_com_"+ str(begin) + ".part"
            fp = open(name, "w")
            fp.write(r)
            fp.close()
            return _i_range(jsondum_path, name, rs-1)
    
    return _i_range(jsondum_path, input_path, rs-1)


#test()

jsondump_path = sys.argv[1]
init_input_path = sys.argv[2]

r_path = i_range(jsondump_path, init_input_path)
print("path:", r_path, ", test number:", t_no)
