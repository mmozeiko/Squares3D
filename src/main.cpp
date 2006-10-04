#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"
#include "version.h"

#ifdef WIN32
#define VC_EXTRALEAN
#include <windows.h>
#endif

int main(int, char* argv[])
{
    STATIC_CHECK(sizeof(wchar_t)==2, DANGER_SIZE_OF_WCHAR_IS_NOT_2);

#ifdef NDEBUG
    std::ofstream log("log.txt");
    clog.rdbuf(log.rdbuf());
#endif

    clog << "*** Squares 3D version: " << g_version << " ***" << endl;

    clog << "Started: " << getDateTime() << endl;

    Random::init();

#ifdef NDEBUG
    try
    {
#endif
        File::init(argv[0]);
#ifdef NDEBUG
        try
        {
#endif
            do
            {
                Game().run();
            }
            while (g_needsToReload);
#ifdef NDEBUG
        }
        catch (string& exception)
        {
            clog << "Exception occured :" << endl
                 << "  " << exception << endl;
#ifdef WIN32
            MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
#endif
        }
#endif
        File::done();
#ifdef NDEBUG
    }
    catch (string& exception)
    {
        clog << "Exception occured :" << endl
             << "  " << exception << endl;
#ifdef WIN32
         MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
#endif
    }
#endif

    clog << "Finished: " << getDateTime() << endl;

#ifndef NDEBUG    
    m_dumpMemoryReport();
#endif
    
    return 0;
}
