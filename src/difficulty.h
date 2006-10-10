#ifndef __DIFFICULTY_H__
#define __DIFFICULTY_H__

#include "common.h"
#include "level.h"
#include "player.h"

class Difficulty : NoCopy
{
public:
    Difficulty();
    void setLevel(int levelToSet)
    {
        m_currentLevel = levelToSet;
    }
    void advanceLevel()
    {
        if (m_currentLevel < 2)
        {
            m_currentLevel++;
        }
    }
    /*
    void getShuffledPlayers(const ProfilesMap* profiles, vector<Player*>* localPlayers)
    {
        playersOut->resize(profiles->size());
        size_t i = 0;
        for each_const(ProfilesMap, *profiles, iter)
        {
            playersOut[i++] = (iter->second;
        }
        std::random_shuffle(playersOut->begin(), playersOut->end());
        playersOut->resize(count);        
    }*/
private:
    int m_currentLevel;
};

#endif
