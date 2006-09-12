#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

#include "common.h"
#include "font.h"

class Vector;
class ScoreMessage;
class ComboMessage;
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
typedef pair<string, int> StringIntPair;

class ScoreBoard : NoCopy
{
public:
    ScoreBoard(Messages* messages);
    void registerPlayer(const string& name);
    int addTotalPoints(const string& name);
    int addPoint(const string& name);
    void incrementCombo(const string& name, const Vector& position);
    int addSelfTotalPoints(const string& name);
    void resetCombo();
    void resetOwnCombo(const string& name);
    void reset();
    void update();
    StringIntPair getMostScoreData();
    

private:
    Scores        m_scores;
    Order         m_playerOrder;
    int           m_joinedCombo;
    Messages*     m_messages;
    bool          m_scoreChanged;
    //TODO: make universal
    Board         m_boardPositions;
    ScoreMessages m_msgVec;
    ComboMessage* m_comboMessage;
};

#endif