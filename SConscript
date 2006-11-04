#
from glob import glob
from os import path

Import("env additional isdebug suffix builddir")

sources = [path.normpath(x) for x in glob("src/*.cpp")]

if not isdebug:
  mmgr = path.normpath("src/mmgr.cpp")
  sources.remove(mmgr)

sources = [x.replace("src", builddir) for x in sources]

env.Program("bin/Squares3D" + suffix, sources + additional)
