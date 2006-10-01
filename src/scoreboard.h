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

struct BoardInfo
{
    BoardInfo(const Vector& pos, const Font::AlignType alignement, const int nextDirY) :
        m_position(pos), m_alignement(alignement), m_nextDirectionY(nextDirY) {}
    Vector          m_position;
    Font::AlignType m_alignement;
    int             m_nextDirectionY;
};

typedef map<string, Account> Scores;
typedef vector<string> Order;
typedef vector<BoardInfo> Board;
typedef vector<ScoreMessage*> ScoreMessages;
typedef vector<ComboMessage*> ComboMessages;
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
    Scores         m_scores;
    Order          m_playerOrder;
    int            m_joinedCombo;
    Messages*      m_messages;
    //TODO: make universal
    Board          m_boardPositions;
    ScoreMessages  m_scoreMessages;
    ComboMessage*  m_comboMessage;
    ComboMessages  m_selfComboMessages;
};

#endif
