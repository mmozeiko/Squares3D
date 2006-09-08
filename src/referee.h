#ifndef __REFEREE_H__
#define __REFEREE_H__

#include "common.h"

class Body;
class Player;
class Ball;
class Messages;
class ScoreBoard;

typedef map<const Body*, std::pair<string, Player*> > BodyToPlayerDataMap;

class Referee : NoCopy
{
public:

    BodyToPlayerDataMap      m_players;
    Body*                    m_ball;
    const Body*              m_ground;
    const Body*              m_lastFieldOwner;
    const Body*              m_lastTouchedObject;
    const Body*              m_lastTouchedPlayer;
    bool                     m_gameOver;

    Referee(Messages* messages, ScoreBoard* scoreBoard);

    void processBallPlayer(const Body* otherBody);
    void processBallGround();
    void registerBall(Ball* ball);
    void registerPlayer(const string& name, Player* player);
    void process(const Body* body1, const Body* body2);
    void manageGame();
    void initEvents();
    void registerBallEvent(const Body* ground, const Body* otherBody);
    void registerPlayerEvent(const Body* player, const Body* other);
    void processPlayerGround(const Body* player);

private:
    ScoreBoard* m_scoreBoard;
    Messages*   m_messages;
};


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

private:
    Scores m_scores;
    int    m_joinedCombo;
};
//  def __init__(self, players):
//    self.players = players
//    self.joinedCombo = 0
//
//  def getTotalPoints(self):
//    return self.joinedCombo
//
//
//  def incrementCombo(self, player, hitMsgList):
//    playerName = player.name
//    self[playerName][1] += 1 #update own combo
//    self.joinedCombo += 1 #update joined combo
//    #renew floating coords
//    hitMsgList.append([self[playerName][1],
//                       1.0,
//                       player.getPosition(),
//                       self.players[playerName].color])
//
//  def addPoint(self, playerName):
//    self[playerName][0] += 1
//    return 1
//
//

#endif
