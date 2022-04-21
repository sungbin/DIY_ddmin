import sys
import os.path
import os
import subprocess
from subprocess import Popen, PIPE, STDOUT

err_msg = "in dump example/jsondump.c:44"

my_env = os.environ.copy()
my_env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"

def test_jsondump(in_str):
    jsondump_path = "./jsondump"

    p = Popen([jsondump_path], stdout=PIPE, stdin=PIPE, stderr=STDOUT, env=my_env)
    p.stdin.write(in_str.encode("utf-8"))
    out = p.communicate()[0].decode("utf-8")

    return err_msg in out

def sub_str(val, in_str):
    my_str = ""
    for dep in range(0, len(in_str)):
        val2 = pow(2, dep)
        if val & val2 >= 1:
            #print(bin(val2))
            #print(in_str[dep])
            my_str += in_str[dep]
    return my_str


in_str = "{\"\"*,s [c]\"\"t}"
s_len = len(in_str)

b_len = pow(2, s_len)
#print(bin(b_len))

#val = 0b1001
#print(sub_str(val,in_str))

for i in range(pow(2, s_len)):
    j = pow(2, s_len) - i
    mtr = sub_str(j, in_str)
    if test_jsondump(mtr):
        print("error:\'"+mtr+'\'')
    #print(mtr)
