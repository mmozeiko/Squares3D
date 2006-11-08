#ifndef __REFEREEBASE_H__
#define __REFEREEBASE_H__

#include "common.h"
#include "vmath.h"
#include "language.h"

class Body;
class Player;
class Ball;
class ScoreBoard;
class Messages;
class Message;
class Sound;
class SoundBuffer;


typedef vector<TextType> FaultsVector;
typedef map<const Body*, Player*> BodyToPlayerMap;

class RefereeBase : public NoCopy
{
public:
    RefereeBase(Messages* messages, ScoreBoard* scoreBoard);
    virtual ~RefereeBase();

    void loadFaultsVector();
    void registerPlayers(const vector<Player*> players);

    virtual void registerBall(Ball* ball) {}
    virtual void process(const Body* body1, const Body* body2) {}
    virtual void update() {}
    virtual void addDelayedProcess(const Body* body1, const Body* body2, float delay) {}

    virtual string getLoserName() const { return ""; }

    void resetOwnCombo(const Body* player);
    void resetCombo();
    void incrementCombo(const Body* player, const Vector& position);
    void scoreBoardCritical(int faultID, const string& name, int points, const Vector& position);

    Sound*        m_sound; //for gameover and other sounds exclusive to referee
    SoundBuffer*  m_soundGameStart;

    bool          m_gameOver;
    Message*      m_over;
    const Body*   m_ground;
    const Body*   m_field;
    const Player* m_humanPlayer;


protected:

    BodyToPlayerMap m_players;
    int             m_matchPoints;
    FaultsVector    m_faultsVector;
    ScoreBoard*     m_scoreBoard;
    Messages*       m_messages;

    SoundBuffer*    m_soundGameOver;
    SoundBuffer*    m_soundFault;
};

#endif
