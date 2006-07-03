#include "config.h"
#include "file.h"
#include "common.h"

Config::Config()
{
    clog << "Reading configuration." << endl;

    File::Reader in(CONFIG_FILE);

    string line;
    while (std::getline(in, line))
    {
        string key, value;
        size_t i = 0;
        size_t size = line.size();

        while (i<size && line[i]==' ') i++;
        while (i<size && line[i]!=' ' && line[i]!='=') key += line[i++];
        while (i<size && line[i]==' ') i++;
        if (i==size || line[i]!='=') continue;
        i++; // =
        while (i<size && line[i]==' ') i++;
        while (i<size && line[i]!=' ') value += line[i++];

        insert(std::make_pair(key, value));
    }

    in.close();
}

Config::~Config()
{
    clog << "Saving configuration." << endl;

    File::Writer out(CONFIG_FILE);
    if (!out.is_open())
    {
        clog << "Error: failed to open " << CONFIG_FILE << " for writing." << endl;
    }
    else
    {
        for each_const(StringMap, *this, items)
        {
            out << items->first << " = " << items->second << endl;
        }
        out.close();
    }
}
