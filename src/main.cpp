#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#endif

#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"
#include "version.h"
#include "audio.h"
#include "video.h"

void display_exception(const string& exception)
{
    clog << "Exception occured :" << endl
         << "  " << exception << endl;
#if defined(WIN32)
    MessageBox(NULL, ("Exception occured (for details see log.txt file):\n" + exception).c_str(), "Squares 3D", MB_ICONERROR);
#elif defined(__APPLE__)
    Str255 err;
    CopyCStringToPascal("Exception occured (for details see log.txt file)", err);
    short x;
    StandardAlert(kAlertStopAlert, err, NULL, NULL, &x);
#else
    std::cout << "Exception occured (for details see log.txt file):" << endl << endl
              << "  " << exception << endl;
#endif
}

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
#ifdef __linux__
            audio_setup();
#endif
#ifdef __APPLE__
            video_setup();
#endif
            do
            {
                Game().run();
            }
            while (g_needsToReload);
#ifdef __linux__
            audio_finish();
#endif
#ifdef __APPLE__
            video_finish();
#endif
        }
        catch (const string& exception)
        {
            display_exception(exception);
        }
        File::done();
    }
    catch (const string& exception)
    {
        display_exception(exception);
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
