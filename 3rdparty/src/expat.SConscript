#
Import("env lib suffix")

headers = Split("""nametab.h utf8tab.h winconfig.h xmlrole.h 
                   xmltok.h xmltok_impl.h ascii.h asciitab.h
                   iasciitab.h internal.h latin1tab.h
                   xmltok_impl.c xmltok_ns.c""")

sources = Split("xmlparse.c xmlrole.c xmltok.c")

p = str( Platform() )

e = env.Copy()

if p == "win32":
  e.Append( CPPDEFINES = ["COMPILED_FROM_DSP"] )

else:
  e.Append( CPPDEFINES = ["HAVE_MEMMOVE"] )

e.StaticLibrary(lib + "/expat" + suffix, ["expat/" + x for x in sources])
