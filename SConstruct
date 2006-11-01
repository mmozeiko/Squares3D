#
import os
from glob import glob

p = str( Platform() )

isdebug = ARGUMENTS.get('debug', 0)

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

    postfix = "_d"
    builddir = "Debug"

  else:
    env.Append( CXXFLAGS   = ["/O2", "/Ot", "/GL", "/FD", "/MT", "/GS-"] )
    env.Append( LINKFLAGS  = ["/LTCG", "/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup", "/OPT:REF", "/OPT:ICF"] )
    env.Append( CPPDEFINES = ["NDEBUG"] )

    postfix = ""
    builddir = "Release"

  additional = env.RES(builddir + "/resource.rc")

  removeMMGR = False

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
    env.Append( CXXFLAGS   = ["-O0", "-g"] )
    env.Append( CXXFLAGS   = ["-D_DEBUG"] )

    postfix = "_d"
    builddir = "Debug"

  else:
    env.Append( CXXFLAGS  = ["-O2"] )
    env.Append( CXXFLAGS  = ["-DNDEBUG"] )

    postfix = ""
    builddir = "Release"

  additional = []

  removeMMGR = True

elif p == "posix":

  env = Environment( tools = ["g++", "ar", "link"], ENV = os.environ )
  env.Append( LIBS = Split("Newton") )
  env.Append( LIBS = Split("GLU GL X11 openal") )

  if isdebug:
    env.Append( CXXFLAGS   = ["-O0", "-g"] )
    env.Append( CXXFLAGS   = ["-D_DEBUG"] )

    postfix = "_d"
    builddir = "Debug"

  else:
    env.Append( CXXFLAGS  = ["-O2"] )
    env.Append( CXXFLAGS  = ["-DNDEBUG"] )

    postfix = ""
    builddir = "Release"

  additional = []

  removeMMGR = True

else:
  raise "Not implemented!"

env.Append(  CXXFLAGS = ["-I3rdparty/include"] )
env.Append(  CXXFLAGS = ["-DXML_STATIC"] )
env.Append(  LIBPATH  = ["3rdparty/lib"] )
env.Prepend( LIBS     = [x+postfix for x in Split("enet expat glfw vorbisfile vorbis ogg physfs zlib")] )

env.BuildDir(builddir, "src", duplicate=0)

sources = [x.replace("src\\", builddir + "\\").replace("src/", builddir+"/") for x in glob("src/*.cpp")]

if not isdebug or removeMMGR:
  for x in ["Release\\mmgr.cpp", "Release/mmgr.cpp",
            "Debug\\mmgr.cpp", "Debug/mmgr.cpp",
            "src\\mmgr.cpp", "src/mmgr.cpp"]:
    if x in sources:
      sources.remove(x)

env.Program("bin/Squares3D" + postfix, sources + additional)
