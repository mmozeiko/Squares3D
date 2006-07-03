#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"

const string CONFIG_FILE = "/config.txt";

class Config : public map<string, string>
{
public:
    Config();
    ~Config();
};

#endif
