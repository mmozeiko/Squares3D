#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"

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

#ifndef NDEBUG    
    m_dumpMemoryReport();
#endif
    
    return 0;
}
