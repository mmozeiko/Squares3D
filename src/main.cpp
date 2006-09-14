#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"

int main(int, char* argv[])
{
    STATIC_CHECK(sizeof(wchar_t)==2, SIZE_OF_WCHAR_IS_NOT_2);

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
            Game().run();
        }
        catch (string& exception)
        {
            clog << "Exception occured :" << endl
                 << "  " << exception << endl;
        }
        File::done();
    }
    catch (string& exception)
    {
        clog << "Exception occured :" << endl
             << "  " << exception << endl;
    }

    clog << "Finished: " << getDateTime() << endl;

#ifndef NDEBUG    
    m_dumpMemoryReport();
#endif
    
    return 0;
}
