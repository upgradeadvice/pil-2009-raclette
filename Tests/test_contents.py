from tester import *

import os, glob

contents = set()
for file in open("CONTENTS"):
    file = file.strip()
    if file:
        contents.add(os.path.normpath(file))

patterns = [
    "PIL/*.py",
    "*.c",
    "libImaging/*.c",
    "libImaging/*.h",
    ]

def test_files():

    for pattern in patterns:
        for file in glob.glob(pattern):
            file = os.path.normpath("Imaging/" + file)
            assert_true(file in contents, "%r not in CONTENTS" % file)

def test_contents():

    for file in contents:
        root, file = file.split(os.sep, 1)
        assert_true(os.path.isfile(file), "%r not found" % file)