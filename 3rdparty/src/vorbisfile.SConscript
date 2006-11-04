#
Import("env lib suffix")

headers = Split("misc.h os.h")
sources = Split("vorbisfile.c")

env.StaticLibrary(lib + "/vorbisfile" + suffix, ["vorbisfile/" + x for x in sources])
