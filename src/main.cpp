#ifdef WIN32
#define VC_EXTRALEAN
#include <windows.h>
#endif

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
    std::ofstream log("log.txt");
    clog.rdbuf(log.rdbuf());
#endif

    clog << "*** Squares 3D version: " << g_version << " ***" << endl;

    clog << "Started: " << getDateTime() << endl;

    Random::init();

    try
    {
        File::init(argv[0]);
        try
        {
            do
            {
                Game().run();
            }
            while (g_needsToReload);
        }
        catch (string& exception)
        {
            clog << "Exception occured :" << endl
                 << "  " << exception << endl;
#ifdef WIN32
            MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
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
#endif
    }

    clog << "Finished: " << getDateTime() << endl;

#if defined(_DEBUG) && !__MINGW32__ && !__APPLE__ && !__linux__
    m_dumpMemoryReport();
#endif
    
    return 0;
}
