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
class Sound;
class SoundBuffer;

typedef map<const Body*, Player*> BodyToPlayerMap;

class Referee : public NoCopy
{
public:

    BodyToPlayerMap          m_players;
    Ball*                    m_ball;
    const Body*              m_ground;
    const Body*              m_field;
    const Body*              m_lastFieldOwner;
    const Body*              m_lastTouchedObject;
    const Body*              m_lastTouchedPlayer;
    const Player*            m_humanPlayer;
    bool                     m_gameOver;

    Referee(Messages* messages, ScoreBoard* scoreBoard);
    ~Referee();

    void registerBall(Ball* ball);
    void registerPlayers(const vector<Player*> players);
    void process(const Body* body1, const Body* body2);
    void update();
    void resetBall();
    void registerBallEvent(const Body* ground, const Body* otherBody);
    bool isGroundObject(const Body* body) const;

    string getLoserName() const;

    bool m_mustResetBall;
    bool m_playersAreHalted;

private:
    void initEvents();
    void haltCpuPlayers(const Player* except = NULL);
    void releaseCpuPlayers();
    Player* getDiagonalPlayer(const Player* player) const;
    void processBallPlayer(const Body* otherBody);
    void processBallGround(const Body* groundObject);
    void registerFaultTime();
    void processCriticalEvent();
    void registerPlayerEvent(const Body* player, const Body* other);
    void processPlayerGround(const Body* player);


    Vector       m_ballResetPosition; 
    Vector       m_ballResetVelocity;
    Timer        m_timer;
    ScoreBoard*  m_scoreBoard;
    Messages*    m_messages;
    int          m_matchPoints;
    int          m_haltWait;
    Player*      m_lastWhoGotPoint;
    Sound*       m_sound; //for gameover and other sounds exclusive to referee
    SoundBuffer* m_soundGameOver;
    SoundBuffer* m_soundGameStart;
    SoundBuffer* m_soundFault;


    Vector      m_lastTouchedPosition;
};

#endif
