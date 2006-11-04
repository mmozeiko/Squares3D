#
Import("env lib suffix")

sources = Split("packet.c peer.c protocol.c host.c list.c memory.c")

p = str( Platform() )

if p == "win32":
  sources += ["win32.c"]

else:
  sources += ["unix.c"]

env.StaticLibrary(lib + "/enet" + suffix, ["enet/" + x for x in sources])
