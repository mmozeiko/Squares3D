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

typedef map<const Body*, Player*> BodyToPlayerMap;

class Referee : NoCopy
{
public:

    BodyToPlayerMap          m_players;
    Body*                    m_ball;
    const Body*              m_ground;
    const Body*              m_field;
    const Body*              m_lastFieldOwner;
    const Body*              m_lastTouchedObject;
    const Body*              m_lastTouchedPlayer;
    const Player*            m_humanPlayer;
    bool                     m_gameOver;

    Referee(Messages* messages, ScoreBoard* scoreBoard);

    void registerBall(Ball* ball);
    void registerPlayers(const vector<Player*> players);
    void process(const Body* body1, const Body* body2);
    void update();
    void resetBall();
    void registerBallEvent(const Body* ground, const Body* otherBody);

    string getLoserName() const;

    bool m_mustResetBall;

private:
    void initEvents();
    void haltCpuPlayers();
    bool isGroundObject(const Body* body);
    void processBallPlayer(const Body* otherBody);
    void processBallGround();
    void registerFaultTime();
    void processCriticalEvent();
    void registerPlayerEvent(const Body* player, const Body* other);
    void processPlayerGround(const Body* player);


    Vector      m_ballResetPosition; 
    Vector      m_ballResetVelocity;
    Timer       m_timer;
    ScoreBoard* m_scoreBoard;
    Messages*   m_messages;
    int         m_matchPoints;
    int         m_waitForGround;
    bool        m_waitForDiagonalPlayerOrGround;

    Vector      m_lastTouchedPosition;
};

#endif
