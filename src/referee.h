#ifndef __REFEREE_H__
#define __REFEREE_H__

#include "common.h"
#include "timer.h"
#include "vmath.h"

class Body;
class Player;
class Ball;
class Messages;
class ScoreBoard;

typedef map<const Body*, Player*> BodyToPlayerDataMap;

class Referee : NoCopy
{
public:

    BodyToPlayerDataMap      m_players;
    Body*                    m_ball;
    const Body*              m_ground;
    const Body*              m_field;
    const Body*              m_lastFieldOwner;
    const Body*              m_lastTouchedObject;
    const Body*              m_lastTouchedPlayer;
    const Player*            m_humanPlayer;
    bool                     m_gameOver;

    Referee(Messages* messages, ScoreBoard* scoreBoard);

    void processBallPlayer(const Body* otherBody);
    void processBallGround();
    void registerBall(Ball* ball);
    void registerPlayers(const vector<Player*> players);
    void process(const Body* body1, const Body* body2);
    void manageGame();
    void initEvents();
    void update();
    void resetBall();
    void registerFaultTime();
    void processCriticalEvent();
    void registerBallEvent(const Body* ground, const Body* otherBody);
    void registerPlayerEvent(const Body* player, const Body* other);
    void processPlayerGround(const Body* player);

private:
    bool isGroundObject(const Body* body);
    
    Vector      m_ballResetPosition; 
    Vector      m_ballResetVelocity;
    bool        m_mustResetBall;
    Timer       m_timer;
    ScoreBoard* m_scoreBoard;
    Messages*   m_messages;
    int         m_matchPoints;
};

#endif
