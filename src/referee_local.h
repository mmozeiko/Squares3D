#ifndef __REFEREELOCAL_H__
#define __REFEREELOCAL_H__

#include "common.h"
#include "timer.h"
#include "vmath.h"
#include "language.h"
#include "referee_base.h"

class Body;
class Player;
class Ball;
class ScoreBoard;
class Message;

typedef vector<pair<pair<const Body*, const Body*>, float> > DelayedProcessesVector;

class RefereeLocal : public RefereeBase
{
public:

    bool          m_playersAreHalted;

    RefereeLocal(Messages* messages, ScoreBoard* scoreBoard);

    void registerBall(Ball* ball);
    void process(const Body* body1, const Body* body2);
    void update();
    void addDelayedProcess(const Body* body1, const Body* body2, float delay);
    bool isGroundObject(const Body* body) const;

    string getLoserName() const;


private:
    bool m_mustResetBall;

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

    void resetOwnCombo(const Body* player);
    void resetCombo();
    void incrementCombo(const Body* player, const Vector& position);
    void scoreBoardCritical(int faultID, const Body* player, int points, const Vector& position);

    Vector       m_ballResetPosition; 
    Vector       m_ballResetVelocity;
    Timer        m_timer;
    int          m_haltWait;
    Player*      m_lastWhoGotPoint;

    Player*      m_releaseTehOne;
    Player*      m_releaseTehOneD;

    Vector       m_lastTouchedPosition;

    DelayedProcessesVector m_delayedProcesses;

};

#endif
