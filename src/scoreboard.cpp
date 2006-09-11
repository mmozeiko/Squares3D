#include "referee.h"
#include "scoreboard.h"
#include "messages.h"
#include "video.h"

Account::Account() : 
    m_total(0),
    m_combo(0)
{
}

ScoreBoard::ScoreBoard(Messages* messages) : 
    m_messages(messages),
    m_scoreChanged(false)
{
    //TODO: make universal
    float fontSize = 32;
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);
    m_boardPositions.push_back(make_pair(Vector(0, 0, 0), Font::Align_Left));
    m_boardPositions.push_back(make_pair(Vector(0, resY - fontSize, 0), Font::Align_Left));
    m_boardPositions.push_back(make_pair(Vector(resX, resY - fontSize, 0), Font::Align_Right));
    m_boardPositions.push_back(make_pair(Vector(resX, 0, 0), Font::Align_Right));
    
    reset();

    m_comboMessage = new ComboMessage(" Hits combo!!", 
                                      Vector(resX / 2, resY / 6, 0),
                                      Vector(0,0,1),
                                      0);
    m_messages->add2D(m_comboMessage);
}

void ScoreBoard::registerPlayer(const string& name)
{
    m_scores[name] = Account();
    m_playerOrder.push_back(name);
}

void ScoreBoard::resetOwnCombo(const string& name)
{
    Account& acc = m_scores[name];
    acc.m_combo = 0;
}

void ScoreBoard::resetCombo()
{
    m_joinedCombo = 0;
    for each_(Scores, m_scores, iter)
    {
        resetOwnCombo(iter->first);
    }
}

void ScoreBoard::reset()
{
    resetCombo();
    for (size_t i = 0; i < m_playerOrder.size(); i++)
    {
        m_scores[m_playerOrder[i]] = Account();
        ScoreMessage* msg = new ScoreMessage(m_playerOrder[i], 
                                m_boardPositions[i].first, 
                                Vector(0,1,0), 
                                m_scores[m_playerOrder[i]].m_total, 
                                m_boardPositions[i].second);
        m_messages->add2D(msg);
        m_msgVec.push_back(msg);
    }
}

int ScoreBoard::addTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += m_joinedCombo;
    m_scoreChanged = true;
    return m_joinedCombo;
}

int ScoreBoard::addPoint(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += 1;
    m_scoreChanged = true;
    return 1;
}

int ScoreBoard::addSelfTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += acc.m_combo;
    m_scoreChanged = true;
    return acc.m_combo;
}

void ScoreBoard::incrementCombo(const string& name)
{
    m_joinedCombo++;
    Account& acc = m_scores.find(name)->second;
    acc.m_combo++;
}

void ScoreBoard::update()
{
    m_comboMessage->m_points = m_joinedCombo;
    if (m_scoreChanged)
    {
        for (size_t i = 0; i < m_playerOrder.size(); i++)
        {
            m_msgVec[i]->m_score = m_scores[m_playerOrder[i]].m_total;
        }
    }
    m_scoreChanged = false;
}
