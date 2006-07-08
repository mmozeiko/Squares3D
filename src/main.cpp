#include <fstream>

#include "file.h"
#include "random.h"
#include "utilities.h"
#include "game.h"

int main(int, char* argv[])
{
    //std::ofstream log("log.txt");
    //clog.rdbuf(log.rdbuf());

    clog << "Started: " << GetDateTime() << endl;

    Random::Init();
    File::Init(argv[0]);

    try
    {
        Game().Run();
    }
    catch (string& exception)
    {
        clog << "Exception occured :" << endl
             << " " << exception << endl;
    }

    File::Done();

    clog << "Finished: " << GetDateTime() << endl;
    
    return 0;
}
