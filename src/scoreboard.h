#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

#include "scoreboard.h"


struct Account
{
    Account();
    int m_total;
    int m_combo;
};

typedef map<string, Account> Scores;

class ScoreBoard : NoCopy
{
public:
    ScoreBoard();
    void registerPlayer(const string& name);
    void addTotalPoints(const string& name);
    void addPoint(const string& name);
    void incrementCombo(const string& name);
    void addSelfTotalPoints(const string& name);
    void resetCombo();
    void reset();
	void ScoreBoard::render();

private:
    Scores    m_scores;
    int       m_joinedCombo;
};

#endif
