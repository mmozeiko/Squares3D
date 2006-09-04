#ifndef __REFEREE_H__
#define __REFEREE_H__

#include "common.h"

class Body;
class Player;

typedef map<Body*, std::pair<string, Player*> > BodyToPlayerDataMap;

class Referee
{
public:

    BodyToPlayerDataMap      m_players;
    Body*                    m_ball;
    Body*                    m_ground;
    Body*                    m_lastFieldOwner;
    Body*                    m_lastTouchedObject;
    Body*                    m_lastTouchedPlayer;
    bool                     m_gameOver;
    bool                     m_afterCollideTriggerBox;
    char                     m_state;

    Referee();

    void processBallPlayer(Body* ball, Body* otherBody);
    void processBallGround(Body* ball, Body* otherBody);
    void registerPlayer(const string& name, Player* player);
    void process(Body* body1, Body* body2);
    void manageGame();
    void initEvents();
    void registerBallEvent(Body* ground, Body* otherBody);
    void registerPlayerEvent(Body* player, Body* other){}
};

class ScoreBoard : public std::map<std::string, std::string>
{

};
//  def __init__(self, players):
//    self.players = players
//    self.joinedCombo = 0
//
//  def getTotalPoints(self):
//    return self.joinedCombo
//
//  def addTotalPoints(self, playerName):
//    pts = self.getTotalPoints()
//    self[playerName][0] += pts #update player score account
//    self.resetJoinedCombo()
//    return pts
//
//  def addSelfPoints(self, playerName):
//    playerAccount = self[playerName]
//    pts = playerAccount[1]
//    playerAccount[0] += pts #update player score account by earned pts
//    self.resetJoinedCombo()
//    return pts
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
//  def resetJoinedCombo(self):
//    for val in self.values(): #reset player combos
//      val[1] = 0 #null them
//    self.joinedCombo = 0 #reset joined as well
//    
//  def reset(self):
//    for playerName in self.players.keys():
//      # (player`s total score (minuses), current combo points
//      self[playerName] = [0, 0]
//    self.joinedCombo = 0
//
//  def resetOwnCombo(self, playerName):
//    self[playerName][1] = 0

#endif
