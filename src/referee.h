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

#endif
