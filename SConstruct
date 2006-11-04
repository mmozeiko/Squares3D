#
import os

p = str( Platform() )

isdebug = ARGUMENTS.get('debug', 0)

if isdebug:
    suffix = "_d"
    builddir = "Debug"

else:
    suffix = ""
    builddir = "Release"

if p == "win32":
  env = Environment( tools = ["masm", "msvc", "msvs", "mslib", "mslink"] )
  env.Append( CXXFLAGS   = ["/EHsc"] )
  env.Append( CPPDEFINES = ["_CRT_SECURE_NO_DEPRECATE", "_SCL_SECURE_NO_DEPRECATE",
                            "WIN32", "_CONSOLE", "XML_STATIC"] )
  env.Append( LINKFLAGS  = ["/INCREMENTAL:NO", "/MANIFEST:NO", "/MACHINE:X86"] )
  env.Append( LIBS       = Split("""advapi32 user32 kernel32
                                    gdi32 winmm opengl32 glu32
                                    ws2_32 Newton OpenAL32""") )

  if isdebug:
    env.Append( CXXFLAGS   = ["/Od", "/MTd", "/EHsc", "/Z7"] )
    env.Append( LINKFLAGS  = ["/SUBSYSTEM:CONSOLE", "/DEBUG", '/PDB:"bin\\Squares3D_d.pdb"'] )
    env.Append( CPPDEFINES = ["_DEBUG"] )

  else:
    env.Append( CXXFLAGS   = ["/O2", "/Ot", "/GL", "/FD", "/MT", "/GS-"] )
    env.Append( LINKFLAGS  = ["/LTCG", "/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup", "/OPT:REF", "/OPT:ICF"] )
    env.Append( CPPDEFINES = ["NDEBUG"] )

  additional = env.RES(builddir + "/resource.rc")

elif p == "darwin":

  env = Environment( tools = ["g++", "ar", "link"], ENV = os.environ )
  env.Append( LIBS      = Split("Newton32") )
  env.Append( CXXFLAGS  = ["-I/System/Library/Frameworks/OpenAL.framework/Versions/Current/Headers"] )
  env.Append( LINKFLAGS = Split("""-framework OpenAL
                                   -framework OpenGL
                                   -framework Carbon
                                   -framework AGL
                                   -framework IOKit""") )

  if isdebug:
    env.Append( CXXFLAGS   = Split("-O0 -g") )
    env.Append( CXXFLAGS   = ["-D_DEBUG"] )

  else:
    env.Append( CCFLAGS   = Split("""-pipe
                                     -Os
                                     -fvisibility=hidden
                                     -fvisibility-inlines-hidden
                                     -mmacosx-version-min=10.4
                                     -mdynamic-no-pic
                                     -isysroot /Developer/SDKs/MacOSX10.4u.sdk""") )
    env.Append( CXXFLAGS  = ["-DNDEBUG"] )
    env.Append( LINKFLAGS = Split("-mmacosx-version-min=10.4") )

  additional = []

elif p == "posix":

  env = Environment( tools = ["g++", "ar", "link"], ENV = os.environ )
  env.Append( LIBS = Split("Newton") )
  env.Append( LIBS = Split("GLU GL X11 openal") )

  if isdebug:
    env.Append( CXXFLAGS   = Split("-O0 -g -pipe") )
    env.Append( CXXFLAGS   = ["-D_DEBUG"] )

  else:
    env.Append( CXXFLAGS  = Split("-O2 -pipe") )
    env.Append( CXXFLAGS  = ["-DNDEBUG"] )

  additional = []

else:
  raise "Not implemented!"

env.Append(  CXXFLAGS = ["-I3rdparty/include"] )
env.Append(  CXXFLAGS = ["-DXML_STATIC"] )
env.Append(  LIBPATH  = ["3rdparty/lib"] )

if p == "darwin":

  e = env.Copy()
  s = suffix
  b = builddir
  
  suffix = s + "_ppc"
  builddir = b + "_ppc"

  env.Append( CCFLAGS   = Split("-arch ppc -mtune=G5") )
  env.Append( LINKFLAGS = Split("-arch ppc") )
  env.Prepend( LIBS     = [x+suffix for x in Split("enet expat glfw vorbisfile vorbis ogg physfs zlib")] )
  env.BuildDir(builddir, "src", duplicate=0)
  SConscript("SConscript", exports="env additional isdebug suffix builddir", duplicate=0)


  env = e
  suffix = s + "_i386"
  builddir = b + "_i386"

  env.Append( CCFLAGS   = Split("-arch i386") )
  env.Append( LINKFLAGS = Split("-arch i386") )
  env.Prepend( LIBS     = [x+suffix for x in Split("enet expat glfw vorbisfile vorbis ogg physfs zlib")] )
  env.BuildDir(builddir, "src", duplicate=0)
  SConscript("SConscript", exports="env additional isdebug suffix builddir", duplicate=0)

  env.Command("bin/Squares3D" + s,
              ["bin/Squares3D" + s + "_i386", "bin/Squares3D" + s + "_ppc"],
              "lipo -create bin/Squares3D%s_i386 bin/Squares3D%s_ppc -output bin/Squares3D%s" % (s, s, s) )
 
else:

  env.Prepend( LIBS     = [x+suffix for x in Split("enet expat glfw vorbisfile vorbis ogg physfs zlib")] )
  SConscript("SConscript", exports="env additional isdebug suffix builddir", duplicate=0)
