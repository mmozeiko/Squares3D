#
Import("env lib suffix")

platformsrc = Split("""enable.c fullscreen.c glext.c init.c
                       joystick.c thread.c time.c window.c""")

headers = Split("internal.h ?!?platform.h")

sources = platformsrc + Split("image.c input.c tga.c")

p = str( Platform() )

e = env.Copy()

if p == "win32":
  platformdir = "win32"
  e.Append( CPPDEFINES = Split("_GLFW_NO_DLOAD_GDI32 _GLFW_NO_DLOAD_WINMM") )

elif p=="darwin":
  platformdir = "macosx"

elif p=="posix":
  platformdir = "x11"
  sources += ["x11/x11_keysym2unicode.c"]
  
  e.Append( CPPDEFINES = Split("_GLFW_HAS_XF86VIDMODE _GLFW_HAS_PTHREAD _GLFW_HAS_GLXGETPROCADDRESS") ) # instead of last define _GLFW_HAS_DLOPEN can be defined

  

else:
  raise "Unsupported platform!"

e.Append( CPPPATH = ["glfw", "glfw/" + platformdir] )

sources += [platformdir + "/" + platformdir + "_" + x for x in platformsrc]

e.StaticLibrary(lib + "/glfw" + suffix, ["glfw/" + x for x in sources])
