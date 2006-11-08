#include "openal_includes.h"

#include "referee_local.h"
#include "player.h"
#include "body.h"
#include "ball.h"
#include "geometry.h"
#include "messages.h"
#include "message.h"
#include "scoreboard.h"
#include "video.h"
#include "profile.h"
#include "random.h"
#include "world.h"
#include "network.h"
#include "colors.h"

static const float BALL_RESET_TIME = 3.0f;
//TODO: make universaly proportional to field size?
static const float lineWeight = 0.15f;

bool isBallInField(const Vector& position, 
                   const Vector& lowerLeft, 
                   const Vector& upperRight,
                   const bool    useMiddleLines = true,
                   const bool    useOutterLines = false)
{
    //just for the real field - excluding middle line

    Vector _lowerLeft(lowerLeft);
    Vector _upperRight(upperRight);

    //find out which of the 4 fields we are in
    Vector center = getSquareCenter(_lowerLeft, _upperRight);

    int quadrant = getQuadrant(center);

    //adjust the field size to take middle line into account
    switch(quadrant)
    {
    case 1: if (useMiddleLines)
            {
                _lowerLeft.x += lineWeight;
                _lowerLeft.z += lineWeight;
            }
            if (useOutterLines)
            {
                _upperRight.x -= lineWeight;
                _upperRight.z -= lineWeight;
            }
            break;
    case 2: if (useMiddleLines)
            {
                _upperRight.x -= lineWeight;
                _lowerLeft.z += lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.x += lineWeight;
                _upperRight.z -= lineWeight;
            }
            break;
    case 3: if (useMiddleLines)
            {
                _upperRight.x -= lineWeight;
                _upperRight.z -= lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.x += lineWeight;
                _lowerLeft.z += lineWeight;
            }
            break;
    case 4: if (useMiddleLines)
            {
                _lowerLeft.x += lineWeight;
                _upperRight.z -= lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.z += lineWeight;
                _upperRight.x -= lineWeight;
            }
            break;
    }

    //return the bool "if point is into adjusted field (middle line excluded)"
    return isPointInRectangle(position, _lowerLeft, _upperRight);
}

RefereeLocal::RefereeLocal(Messages* messages, ScoreBoard* scoreBoard) :
    RefereeBase(messages, scoreBoard),
    m_ball(NULL),
    m_lastFieldOwner(NULL),
    m_lastTouchedPlayer(NULL),
    m_lastWhoGotPoint(NULL),
    m_mustResetBall(false),
    m_timer(),
    m_haltWait(3),
    m_playersAreHalted(false),
    m_releaseTehOne(NULL),
    m_releaseTehOneD(NULL)
{
    m_humanPlayer = NULL;
    m_gameOver = false;
    m_over = NULL;
    m_ground = NULL;
    m_field = NULL;
    initEvents();
}

string RefereeLocal::getLoserName() const
{
    return m_scoreBoard->getMostScoreData().first;
}
bool RefereeLocal::isGroundObject(const Body* body) const
{
    return ((body == m_ground) || (body == m_field));
}

void RefereeLocal::registerFaultTime()
{
    m_timer.reset();
    m_mustResetBall = true;
}

void RefereeLocal::update()
{
    if (m_mustResetBall && !m_gameOver)
    {
        if (m_timer.read() > BALL_RESET_TIME)
        {
            resetBall();
            m_mustResetBall = false;
        }
    }
    updateDelayedProcesses();
}

Player* RefereeLocal::getDiagonalPlayer(const Player* player) const
{
    Vector dFieldCenter;
    Player* returnPtr = NULL;
    dFieldCenter.x = player->getFieldCenter().x * -1;
    dFieldCenter.z = player->getFieldCenter().z * -1;
    for each_const(BodyToPlayerMap, m_players, iter)
    {
        if (iter->second->getFieldCenter() == dFieldCenter)
        {
            returnPtr = iter->second;
            break;
        }
    }
    assert(returnPtr != NULL);
    return returnPtr;
}

void RefereeLocal::haltCpuPlayers(const Player* except)
{
    m_playersAreHalted = true;
    for each_const(BodyToPlayerMap, m_players, iter)
    {
        if (iter->second != except)
        {
            iter->second->halt();
        }
        else
        {
            //except can go even if he was halted
            iter->second->release();
        }
    }
}

void RefereeLocal::releaseCpuPlayers()
{
    m_haltWait = 0;
    m_playersAreHalted = false;
    for each_const(BodyToPlayerMap, m_players, iter)
    {
        if (iter->second != m_releaseTehOne)
        {
            iter->second->release();
        }
    }
}

void RefereeLocal::releaseCpuPlayer(Player* player)
{
    player->release();
}

void RefereeLocal::resetBall()
{
    float random = static_cast<float>(Randoms::getIntN(2) + 1) / 50;
    Vector resetPosition(random, 1.8f, random);
    Vector velocity = Vector::Zero;

    if ((m_lastWhoGotPoint != NULL)
        || (m_lastTouchedObject != NULL)
        || (m_lastTouchedPlayer != NULL)
        || (m_lastFieldOwner != NULL))
    {
        //the game was in progress
        if (m_lastTouchedObject == NULL)
        {
            //middle line -> reset coords in center
            resetPosition.y *= 3;
            haltCpuPlayers();
            m_haltWait = 1;
        }
        else
        {
            Vector center;
            if (isGroundObject(m_lastTouchedObject))
            {
                //ball has left game field from one of players fields
                //also happens when player touches twice
                //reset from last owner (m_lastFieldOwner)
                center = m_lastWhoGotPoint->getFieldCenter();
            }
            else
            {
                //ball has left game field from one of the players
                //also happens on preliminary touches
                //reset from m_lastTouchedPlayer
                center = m_lastWhoGotPoint->getFieldCenter();
            }
            //set the reset position to center of players field
            resetPosition = Vector(center.x, resetPosition.y, center.z);
            velocity = (Vector::Zero - resetPosition) * 2;
            Player* exceptPlayer = getDiagonalPlayer(m_lastWhoGotPoint);
            haltCpuPlayers(); //exceptPlayer);
            m_haltWait = 2;
            m_releaseTehOne = exceptPlayer;
            m_releaseTehOneD = m_lastWhoGotPoint;
        }
    }
    else
    {
        //the game has just begun
        //reset coords in center and ball must hit the ground 3 times
        //before it can be touched by players

        resetPosition.y *= 3;
        m_haltWait = 3;
        haltCpuPlayers();
    }

    m_ball->m_body->setTransform(resetPosition, Vector::Zero);
    NewtonBodySetOmega(m_ball->m_body->m_newtonBody, Vector::Zero.v);
    NewtonBodySetVelocity(m_ball->m_body->m_newtonBody, velocity.v);
    initEvents();
}

void RefereeLocal::initEvents()
{
    m_lastFieldOwner    = NULL;
    m_lastTouchedObject = NULL;
    m_lastTouchedPlayer = NULL;
    m_lastTouchedPosition = Vector::Zero;
}

void RefereeLocal::processCriticalEvent()
{
    StringIntPair maxScore = m_scoreBoard->getMostScoreData();
    if (maxScore.second >= m_matchPoints)
    {
        string overText;
        if (maxScore.first == m_humanPlayer->m_profile->m_name)
        {
            overText = Language::instance->get(TEXT_RESTART);
        }
        else
        {
            if (World::instance->m_current == 3)
            {
                overText = Language::instance->get(TEXT_FINISHED_GAME);
            }
            else
            {
                overText = Language::instance->get(TEXT_CONTINUE);
            }
        }
            
        Vector center = Vector(static_cast<float>(Video::instance->getResolution().first) / 2,
                               static_cast<float>(Video::instance->getResolution().second) / 2,
                               0.0f);

        if (Network::instance->m_isSingle)
        {
            m_over = new Message(overText, 
                                 Vector(center.x, center.y - (72 + 32), center.z), 
                                 brighter(Grey, 3.0f), 
                                 Font::Align_Center,
                                 32);
            m_messages->add2D(m_over);
        }

        m_gameOver = true;

        for each_const(BodyToPlayerMap, m_players, iter)
        {
            iter->second->halt();
        }
    }
    else
    {
        registerFaultTime();
    }

    m_delayedProcesses.clear();
}

void RefereeLocal::registerBall(Ball* ball)
{
    ball->m_referee = this;
    m_ball = ball;
    resetBall();
}

void RefereeLocal::process(const Body* body1, const Body* body2)
{
    if (body1 == m_ball->m_body)
    {
        registerBallEvent(body1, body2);
    }
    if (foundIn(m_players, body1))
    {
        registerPlayerEvent(body1, body2);
    }
}

void RefereeLocal::addDelayedProcess(const Body* body1, const Body* body2, float delay)
{
    m_delayedProcesses.push_back(make_pair(make_pair(body1, body2), m_timer.read() + delay));
}

void RefereeLocal::updateDelayedProcesses()
{
    DelayedProcessesVector::iterator iter = m_delayedProcesses.begin();
    float curTime = m_timer.read();
    while (iter != m_delayedProcesses.end())
    {
        if (curTime > iter->second)
        {
            process(iter->first.first, iter->first.second);
            if (m_delayedProcesses.empty())
            {
                // if critical process has cleared m_delayedProcesses - break 
                break;
            }

            iter = m_delayedProcesses.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}


void RefereeLocal::registerPlayerEvent(const Body* player, const Body* otherBody)
{
    if (isGroundObject(otherBody))
    {
        //TODO: play some step sound here?
        if (!(m_gameOver || m_mustResetBall))
        {
            processPlayerGround(player);
        }
    }
}

void RefereeLocal::registerBallEvent(const Body* ball, const Body* otherBody)
{
   if (isGroundObject(otherBody))
    {
        if (!(m_gameOver || m_mustResetBall))
        {
            processBallGround(otherBody);
        }
    }
    else if (foundIn(m_players, otherBody))
    {
        if (!(m_gameOver || m_mustResetBall))
        {
            processBallPlayer(otherBody);
        }
    }
}

void RefereeLocal::processPlayerGround(const Body* player)
{
    //if (Network::instance->m_isServer)
    //{
    //    Network::instance->addPacketToBuffer(player, m_ground);
    //}

    Vector playerPos = player->getPosition();
    string currentPlayerName = player->m_id;
    Player* currentPlayer = m_players.find(player)->second;

    if (!isPointInRectangle(playerPos, currentPlayer->m_lowerLeft, currentPlayer->m_upperRight)
        && isPointInRectangle(playerPos, 
                              Vector(-FIELD_LENGTH, 0, -FIELD_LENGTH),
                              Vector( FIELD_LENGTH, 0,  FIELD_LENGTH)))
    {
        //clog << "pleijeris " + currentPlayerName + " ir iekaapis cita pleijera laukumaa!!.." << endl;
    }
}

void RefereeLocal::processBallGround(const Body* groundObject)
{
    //if (Network::instance->m_isServer)
    //{
    //    Network::instance->addPacketToBuffer(m_ball->m_body, groundObject);
    //}


    if (m_playersAreHalted && (groundObject == m_field))
    {
        m_haltWait--;
        if (m_haltWait == 0)
        {
            releaseCpuPlayers();
        }
        if (m_haltWait == 1 && m_releaseTehOne != NULL)
        {
            releaseCpuPlayer(m_releaseTehOne);
            m_releaseTehOne = NULL;
        }
    }
    else if (!m_playersAreHalted && (groundObject == m_field) && m_releaseTehOneD != NULL)
    {
        releaseCpuPlayer(m_releaseTehOneD);
        m_releaseTehOneD = NULL;
    }


    Vector ballPos(m_ball->getPosition());

    if (!isPointInRectangle(ballPos, 
                            Vector(-FIELD_LENGTH, 0.0f, -FIELD_LENGTH), 
                            Vector( FIELD_LENGTH, 0.0f,  FIELD_LENGTH))) 
    {
        //critical event
        //BALL HAS HIT THE FIELD OUTSIDE
        //and we handle score here

        if (m_lastTouchedObject != NULL) //if last touched was not middle line
        {
            if (foundIn(m_players, m_lastTouchedObject))
            {
                //player has kicked the ball out
                int points = m_scoreBoard->getSelfTotalPoints(m_lastTouchedObject->m_id);
                
                scoreBoardCritical(0, m_lastTouchedObject, points, m_ball->getPosition());

                m_lastWhoGotPoint = m_players.find(m_lastTouchedObject)->second;
            }

            else if (m_lastFieldOwner != NULL) //if ground was touched in one of the players field last add points to owner
            {
                string owner = m_lastFieldOwner->m_id;
                
                int points = 1;

                if (m_lastTouchedPlayer != NULL)
                {
                    if (m_lastFieldOwner == m_lastTouchedPlayer)
                    {
                        points = m_scoreBoard->getSelfTotalPoints(owner);
                    }
                    else
                    {
                        points = m_scoreBoard->getTotalPoints(owner);
                    }
                }
                else
                {
                    //case when noone touched the ball after throwing minus
                }

                scoreBoardCritical(1, m_lastFieldOwner, points, m_ball->getPosition());

                m_lastWhoGotPoint = m_players.find(m_lastFieldOwner)->second;
            }
        }
        else
        {
            //middle line
            scoreBoardCritical(2, NULL, 0, m_ball->getPosition());
        }

        //finishing handling critical branch
        processCriticalEvent();
    }
    else
    {
        //non critical event - update events status
        //BALL HAS HIT THE FIELD INSIDE
        //save the touched field
        for each_const(BodyToPlayerMap, m_players, player)
        {
            //field excluding middle line
            if (isBallInField(ballPos, 
                              player->second->m_lowerLeft, 
                              player->second->m_upperRight))
            {
               m_lastFieldOwner =    player->first;
               m_lastTouchedObject = m_ground;
               break;
            }
            m_lastTouchedObject = NULL; //if has hit the middle line
        }
    }
}

void RefereeLocal::processBallPlayer(const Body* player)
{
    //if (Network::instance->m_isServer)
    //{
    //    Network::instance->addPacketToBuffer(m_ball->m_body, player);
    //}


    //ball + player

    string playerName = player->m_id;

    //look if players are obeying rules after the ball reset
    if (m_playersAreHalted)
    {
        if ((m_lastTouchedObject == NULL) 
            || (m_players.find(player)->second != getDiagonalPlayer(m_lastWhoGotPoint)))
        {
            //unnallowed action - critical event
            //either player touched the ball after the throw in middle line,
            //but the ball hasn`t touched the ground enough times
            //or player was not allowed to touch the ball after the fault throw-in

            int points = 1;
            scoreBoardCritical(4, player, points, m_ball->getPosition());

            m_lastTouchedObject = player;
            m_lastTouchedPlayer = player;
            m_lastWhoGotPoint = m_players.find(player)->second;
            processCriticalEvent();
            goto end;
        }
        else
        {
            //all ok - player was on diagonal and was allowed to touch
            releaseCpuPlayers();
        }
    }


    if (m_lastTouchedObject == NULL) // last object is neither ground nor player,
    {
        resetCombo(); //resetting combo in case picked from middle line
        incrementCombo(player, m_ball->getPosition());
    }
    else if (isGroundObject(m_lastTouchedObject)) // picked from ground inside
    {
        resetCombo(); //clear combo

        incrementCombo(player, m_ball->getPosition());

        if ((m_lastFieldOwner == player)
            && (m_lastTouchedPlayer == player)
            
            && isBallInField(m_ball->getPosition(), 
                              m_players.find(player)->second->m_lowerLeft, 
                              m_players.find(player)->second->m_upperRight,
                              true, true)

            && isPointInRectangle(player->getPosition(), 
                              m_players.find(player)->second->m_lowerLeft, 
                              m_players.find(player)->second->m_upperRight)

            && isPointInRectangle(m_lastTouchedPosition,
                              m_players.find(player)->second->m_lowerLeft, 
                              m_players.find(player)->second->m_upperRight)
            )
        {
            //critical event. double-touched -> fault

            int points = 1;
            scoreBoardCritical(3, player, points, m_ball->getPosition());

            m_lastWhoGotPoint = m_players.find(player)->second;
            processCriticalEvent();
            //TODO: restructure ifs?
            goto end;
        }
    }
    else //picked from player
    {
        if (m_lastTouchedPlayer != player) //reset own combo, when picked from other
        {        
            resetOwnCombo(player);
        }
        incrementCombo(player, m_ball->getPosition()); //(+1)
    }
    
    m_lastTouchedObject = player;
    m_lastTouchedPlayer = player;
    m_lastTouchedPosition = player->getPosition();
end:;
}

void RefereeLocal::resetOwnCombo(const Body* player)
{
    Network::instance->addResetOwnComboPacket(player);
    m_scoreBoard->resetOwnCombo(player->m_id);
}

void RefereeLocal::resetCombo()
{
    Network::instance->addResetComboPacket();
    m_scoreBoard->resetCombo();
}

void RefereeLocal::incrementCombo(const Body* player, const Vector& position)
{
    Network::instance->addIncrementComboPacket(player);
    m_scoreBoard->incrementCombo(player->m_id, m_players.find(player)->second->m_profile->m_color, position); //(+1)
}

void RefereeLocal::scoreBoardCritical(int faultID, const Body* player, int points, const Vector& position)
{
    Network::instance->addRefereePacket(faultID, player, points);
    string name;
    if (player == NULL)
    {
        //middle line it was then
        name = "";
    }
    else
    {
        name = player->m_id;
    }
    RefereeBase::scoreBoardCritical(faultID, name, points, position);
}
