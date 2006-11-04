#
Import("env lib suffix")

sources = Split("bitwise.c framing.c")

env.StaticLibrary(lib + "/ogg" + suffix, ["ogg/" + x for x in sources])
