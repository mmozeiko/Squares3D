#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <Carbon/Carbon.h>
#endif

#include <GL/glfw.h>
#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"
#include "version.h"

int main(int, char* argv[])
{
#ifdef NDEBUG
    std::ofstream log((File::getBase(argv[0], true) +  "log.txt").c_str());
    std::streambuf* old_clog = clog.rdbuf(log.rdbuf());
#endif

    clog << "*** Squares 3D version: " << g_version << " ***" << endl;

    clog << "Started: " << getDateTime() << endl;

    Randoms::init();

    try
    {
        File::init(argv[0]);
        try
        {
            if (glfwInit() != GL_TRUE)  // MacOSX bug!!
            {
                throw Exception("glfwInit failed");
            }
            do
            {
                Game().run();
            }
            while (g_needsToReload);
            glfwTerminate(); // MacOSX bug!!
        }
        catch (string& exception)
        {
            clog << "Exception occured :" << endl
                 << "  " << exception << endl;
#if defined(WIN32)
            MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
#elif defined(__APPLE__)
            string err(" Exception occured (for details see log.txt file");
            err[0] = static_cast<char>(err.size()+1);
            short x;
            StandardAlert(kAlertStopAlert, err.c_str(), NULL, NULL, &x);
#else
            std::cout << "Exception occured (for details see log.txt file):" << endl << endl;
#endif
        }
        File::done();
    }
    catch (string& exception)
    {
        clog << "Exception occured :" << endl
             << "  " << exception << endl;
#ifdef WIN32
         MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
#elif defined(__APPLE__)
            string err(" Exception occured (for details see log.txt file");
            err[0] = static_cast<char>(err.size()+1);
            short x;
            StandardAlert(kAlertStopAlert, err.c_str(), NULL, NULL, &x);
#else
            std::cout << "Exception occured (for details see log.txt file):" << endl << endl;
#endif
    }

    clog << "Finished: " << getDateTime() << endl;

#if defined(_DEBUG) && !__MINGW32__ && !__APPLE__ && !__linux__
    m_dumpMemoryReport();
#endif
    
#ifdef NDEBUG
    clog.rdbuf(old_clog);
    log.close();
#endif

    return 0;
}
