#
Import("env lib suffix")

headers = Split("physfs_internal.h")

sources = Split("zip.c dir.c physfs.c physfs_byteorder.c")

p = str( Platform() )

e = env.Copy()

if p == "win32":
  sources += ["win32.c"]

elif p=="darwin":
  sources += Split("unix.c posix.c")

else:
  sources += Split("unix.c posix.c")

e.Append( CPPDEFINES = Split("PHYSFS_SUPPORTS_ZIP PHYSFS_NO_CDROM_SUPPORT") )

e.StaticLibrary(lib + "/physfs" + suffix, ["physfs/" + x for x in sources])
