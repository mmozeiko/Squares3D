#include "referee_base.h"
#include "scoreboard.h"
#include "messages.h"
#include "message.h"
#include "player.h"
#include "colors.h"
#include "audio.h"
#include "sound.h"
#include "video.h"
#include "profile.h"

RefereeBase::~RefereeBase()
{
    Audio::instance->unloadSound(m_soundGameOver);
    Audio::instance->unloadSound(m_soundGameStart);
    Audio::instance->unloadSound(m_soundFault);
        
    delete m_sound;
}

RefereeBase::RefereeBase(Messages* messages, ScoreBoard* scoreBoard) :
    m_scoreBoard(scoreBoard),
    m_messages(messages),
    m_sound(new Sound(true)),
    m_matchPoints(21),
    m_gameOver(false),
    m_over(NULL),
    m_ground(NULL),
    m_field(NULL),
    m_humanPlayer(NULL)
{
    loadFaultsVector();

    m_soundGameOver = Audio::instance->loadSound("referee_game_over");
    m_soundGameStart = Audio::instance->loadSound("referee_game_start");
    m_soundFault = Audio::instance->loadSound("referee_fault");

    m_sound->update(Vector::Zero, Vector::Zero);

}

void RefereeBase::scoreBoardCritical(int faultID, const string& name, int points, const Vector& position)
{
    if (points != 0)
    {
        //if there is something to add (not middle line or whatever)
        m_scoreBoard->addPoints(name, points);
    }

    m_messages->add3D(new FlowingMessage(
        Language::instance->get(
                            m_faultsVector[faultID])
                            (name)
                            (points),
        position,
        Red,
        Font::Align_Center));

    int maxScore = m_scoreBoard->getMostScoreData().second;
    if (maxScore >= m_matchPoints)
    {
        m_sound->play(m_soundGameOver);

        Vector center = Vector(static_cast<float>(Video::instance->getResolution().first) / 2,
                               static_cast<float>(Video::instance->getResolution().second) / 2,
                               0.0f);
        m_messages->add2D(new BlinkingMessage(Language::instance->get(TEXT_GAME_OVER), 
                                              center, 
                                              Red, 
                                              Font::Align_Center,
                                              72,
                                              0.8f));
    }
    else
    {
        m_sound->play(m_soundFault);
    }
    m_scoreBoard->resetCombo();
}

void RefereeBase::loadFaultsVector()
{
    m_faultsVector.push_back(TEXT_PLAYER_KICKS_OUT_BALL);
    m_faultsVector.push_back(TEXT_OUT_FROM_FIELD);
    m_faultsVector.push_back(TEXT_OUT_FROM_MIDDLE_LINE);
    m_faultsVector.push_back(TEXT_PLAYER_TOUCHES_TWICE);
    m_faultsVector.push_back(TEXT_PLAYER_UNALLOWED);
}

void RefereeBase::registerPlayers(const vector<Player*> players)
{
    for each_const(vector<Player*>, players, iter)
    {
        Player* player = *iter;
        m_players[player->m_body] = *iter;
        player->m_referee = this;
        
        player->m_body->m_soundable = true;
        player->m_body->m_important = true;
    }
    m_scoreBoard->registerPlayers(players);
}

void RefereeBase::resetOwnCombo(const Body* player)
{
    m_scoreBoard->resetOwnCombo(player->m_id);
}

void RefereeBase::resetCombo()
{
    m_scoreBoard->resetCombo();
}

void RefereeBase::incrementCombo(const Body* player, const Vector& position)
{
    m_scoreBoard->incrementCombo(player->m_id, m_players.find(player)->second->m_profile->m_color, position); //(+1)
}
