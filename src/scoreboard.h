#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

#include "font.h"

class Vector;
class ScoreMessage;
class Messages;

struct Account
{
    Account();
    int m_total;
    int m_combo;
};

typedef map<string, Account> Scores;
typedef vector<string> Order;
typedef vector<pair<Vector, Font::AlignType> > Board;
typedef vector<ScoreMessage*> ScoreMessages;

class ScoreBoard : NoCopy
{
public:
    ScoreBoard(Messages* messages);
    void registerPlayer(const string& name);
    void addTotalPoints(const string& name);
    void addPoint(const string& name);
    void incrementCombo(const string& name);
    void addSelfTotalPoints(const string& name);
    void resetCombo();
    void resetOwnCombo(const string& name);
    void reset();
    void update();

private:
    Scores        m_scores;
    Order         m_playerOrder;
    int           m_joinedCombo;
    Messages*     m_messages;
    bool          m_scoreChanged;
    //TODO: make universal
    Board         m_boardPositions;
    ScoreMessages m_msgVec;
};

#endif
