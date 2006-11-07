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
class Message;

typedef map<const Body*, Player*> BodyToPlayerMap;
typedef vector<pair<pair<const Body*, const Body*>, float> > DelayedProcessesVector;


class Referee : public NoCopy
{
public:

    bool          m_gameOver;
    Message*      m_over;
    const Body*   m_ground;
    const Body*   m_field;
    const Player* m_humanPlayer;
    bool          m_playersAreHalted;

    Sound*        m_sound; //for gameover and other sounds exclusive to referee
    SoundBuffer*  m_soundGameStart;

    Referee(Messages* messages, ScoreBoard* scoreBoard);
    ~Referee();

    bool isGroundObject(const Body* body) const;

    void registerBall(Ball* ball);
    void registerPlayers(const vector<Player*> players);
    void process(const Body* body1, const Body* body2);
    void update();
    void addDelayedProcess(const Body* body1, const Body* body2, float delay);

    string getLoserName() const;


private:
    bool m_mustResetBall;

    BodyToPlayerMap m_players;
    Ball*           m_ball;
    const Body*     m_lastFieldOwner;
    const Body*     m_lastTouchedObject;
    const Body*     m_lastTouchedPlayer;

    void initEvents();
    void resetBall();
    void haltCpuPlayers(const Player* except = NULL);
    void releaseCpuPlayers();
    void releaseCpuPlayer(Player* player);
    Player* getDiagonalPlayer(const Player* player) const;
    void registerBallEvent(const Body* ground, const Body* otherBody);
    void processBallPlayer(const Body* otherBody);
    void processBallGround(const Body* groundObject);
    void registerFaultTime();
    void processCriticalEvent();
    void registerPlayerEvent(const Body* player, const Body* other);
    void processPlayerGround(const Body* player);
    void updateDelayedProcesses();

    Vector       m_ballResetPosition; 
    Vector       m_ballResetVelocity;
    Timer        m_timer;
    ScoreBoard*  m_scoreBoard;
    Messages*    m_messages;
    int          m_matchPoints;
    int          m_haltWait;
    Player*      m_lastWhoGotPoint;
    SoundBuffer* m_soundGameOver;
    SoundBuffer* m_soundFault;

    Player*      m_releaseTehOne;
    Player*      m_releaseTehOneD;

    Vector      m_lastTouchedPosition;

    DelayedProcessesVector m_delayedProcesses;
};

#endif
