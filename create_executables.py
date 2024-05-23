"""
Edits `demos/sha2_claw_demo.cpp` macros specifically, NBYTES_A, NBYTES_B,
NBYTES_C, for values in the range (1, 4).
"""

import os
# import itertools as itr
# from tqdm import tqdm
import subprocess
import time
import math
import random
import argparse


# Read the problem size from the command line


bits_range = list(range(16, 25))
# let's focus when they are equal
all_triples = [(i, i, i) for i in bits_range]  # itr.product(bytes_range, repeat=3)
difficulty_range = 48  # i.e. difficulty between 0 and difficulty_range included


def edit_collision_demo(triple):
    """Replace the nbytes_c, nbytes_b, nbytes_a from."""
    keywords = ["NBITS_A", "NBITS_B", "NBITS_C"]

    with open("demos/sha2_collision_demo.cpp", "r") as claw,\
         open("demos/tmp_col.cpp", "w") as tmp:
        for line in claw:
            for i in range(len(keywords)):
                if line.startswith("#define " + keywords[i]):
                    line = "#define " + keywords[i] + " " + str(triple[i])\
                         + " \n"
            tmp.write(line)

    os.replace("demos/tmp_col.cpp", "demos/sha2_collision_demo.cpp")


def print_errors_if_any(result):
    """Print errors if any from subprocess run."""
    if result.stderr:
        print("Error:", result.stderr.decode())


def compile_project():
    """Compile the project."""
    clean_cmake = "rm -rf CMakeCache.txt  CMakeFiles/"
    compile_cmd = "cmake -S . -B . -D AES_IMPL=aesni && make"

    result = subprocess.run(clean_cmake, shell=True,
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    print_errors_if_any(result)

    result = subprocess.run(compile_cmd, shell=True,
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    print_errors_if_any(result)


def rename_executable(nbits_A_C, difficulty):
    move_cmd = f"cp sha2_collision_demo sha2_collision_demo_{nbits_A_C}_{difficulty}"
    result = subprocess.run(move_cmd, shell=True,
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    print_errors_if_any(result)






# First compile all the triples sequentially
for triple in all_triples:
    print(f"Triple: {triple}")
    edit_collision_demo(triple)
    compile_project()

    nbits_C = triple[-1]
    difficulty_range = (nbits_C)
    for difficulty in range(difficulty_range):
        rename_executable(nbits_C, difficulty)

    # for difficulty in range(min(difficulty_range + 1, nbits_C)):
    #     print(f"difficulty={difficulty}, (|C|, |A|, |B|)={triple}")
    #     try:
    #         for _ in tqdm(range(nruns)):
    #             run_project(int(nbits_C/2), difficulty)
    #     except:
    #         print("Think about installing tqdm by pip install tqdm")
    #         for _ in range(nruns):
    #             run_project(difficulty)



