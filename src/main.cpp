#include <fstream>

#include "common.h"
#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"

int main(int, char* argv[])
{
    //std::ofstream log("log.txt");
    //clog.rdbuf(log.rdbuf());

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
