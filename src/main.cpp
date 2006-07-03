#include <fstream>

#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"
#include "common.h"

int main(int, char* argv[])
{
    //std::ofstream log("log.txt");
    //clog.rdbuf(log.rdbuf());

    clog << "Started: " << getDateTime() << endl;

    Random::init();
    File::init(argv[0]);

    try
    {
        Game().run();
    }
    catch (string& exception)
    {
        clog << "Exception occured :" << endl
             << " " << exception << endl;
    }

    File::done();

    clog << "Finished: " << getDateTime() << endl;
    
    return 0;
}
