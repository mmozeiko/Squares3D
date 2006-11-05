#
Import("env lib suffix")

sources = Split("GLee.c")

env.StaticLibrary(lib + "/glee" + suffix, ["glee/" + x for x in sources])
