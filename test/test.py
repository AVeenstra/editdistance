import math
import os
import Levenshtein
import subprocess


def command_success(ret: subprocess.CompletedProcess):
    if ret.returncode:
        print(ret.returncode)
        print(ret.args)
        if ret.stdout:
            print(ret.stdout)
        exit(1)
    return ret

step = 8589934592
max_size = 14*step+1

extra = [
    3000000000,
    1500000000,
    6000000000,
    7000000000,
   11000000000,
   12000000000,
   13000000000,
   14000000000,
]

groups = ["ave,slb", "ave", "slb"]
results = "/home/veenstraaj/results"
result_name = "r{}{}{}"
command_base = ["mpirun", "--host", "", "-wdir", "/home/veenstraaj/CLionProjects/edit_distance_code/cmake-build-release/", "./editdistance", "--"]
seq_gen_base = ["mpirun", "--host", groups[0], "-wdir", "/home/veenstraaj/", "/usr/bin/python3", "/home/veenstraaj/test_sub.py", str(step), str(max_size)] + list(str(x) for x in extra)
copy_command = ["scp", "/home/veenstraaj/test_sub.py", "{}:/home/veenstraaj/"]
seq_root = "/home/veenstraaj/CLionProjects/edit_distance_code/cmake-build-release/sequences"

for g in groups[2:]:
    cmd = copy_command[:]
    cmd[2] = cmd[2].format(g)
    command_success(subprocess.run(cmd, stdout=subprocess.PIPE))
command_success(subprocess.run(seq_gen_base, stdout=subprocess.PIPE))

files = os.listdir(results)
files = filter(lambda x: x.startswith('r') and x.endswith('control'), files)
files = list(int(x[1:-len('control')]) for x in files)
#print(list((math.sqrt(x),x) for x in sorted(list(set(files)))))
#exit()

#while size <= max_size:
for size in set(list(range(step, max_size, step)) + extra + files):
    for g in groups:
        cmd = command_base[:]
        cmd[2] = g
        cmd.append("sequences/seqa{}".format(size))
        cmd.append("sequences/seqb{}".format(size))
        for i in range(7):
            target_name = result_name.format(size, g.replace(',', '_'), i)
            target = os.path.join(results, target_name)
            if not os.path.isfile(target):
                print(target_name)
                result = command_success(subprocess.run(cmd, stdout=subprocess.PIPE)).stdout
                file = open(target, "wb")
                file.write(result)
                file.close()
    target_name = result_name.format(size, 'control', '')
    target = os.path.join(results, target_name)
    if not os.path.isfile(target):
        print(target_name)
        a = open(os.path.join(seq_root, "seqa{}".format(size)), 'rb').read()
        b = open(os.path.join(seq_root, "seqb{}".format(size)), 'rb').read()
        file = open(target, "w")
        file.write("Result: {}\n".format(str(Levenshtein.distance(a,b))))
        file.close()
