#include "scoreboard.h"
#include "referee.h"
#include "messages.h"
#include "message.h"
#include "video.h"
#include "language.h"

Account::Account() : 
    m_total(0),
    m_combo(0)
{
}

ScoreBoard::ScoreBoard(Messages* messages) : 
    m_messages(messages)
{
    //TODO: make universal
    float fontSize = 32;
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);
    float tabX = resX / 70;
    float tabY = resY / 100;

    m_boardPositions.push_back(BoardInfo(Vector(tabX, tabY, 0), Font::Align_Left, 1));
    m_boardPositions.push_back(BoardInfo(Vector(tabX, resY - fontSize - tabY, 0), Font::Align_Left, -1));
    m_boardPositions.push_back(BoardInfo(Vector(resX - tabX, resY - fontSize - tabY, 0), Font::Align_Right, -1));
    m_boardPositions.push_back(BoardInfo(Vector(resX - tabX, tabY, 0), Font::Align_Right, 1));
    
    reset();

    m_comboMessage = new ComboMessage(Language::instance->get(TEXT_HITS_COMBO), 
                                      Vector(resX / 2, 50.0f, 0),
                                      Vector(1,1,0),
                                      0,
                                      Font::Align_Center,
                                      32);
    m_messages->add2D(m_comboMessage);
}

void ScoreBoard::registerPlayer(const string& name)
{
    m_scores[name] = Account();
    m_playerOrder.push_back(name);
}

StringIntPair ScoreBoard::getMostScoreData()
{
    int max = -1;
    string name;
    for each_const(Scores, m_scores, score)
    {
        if (score->second.m_total > max)
        {
            max = score->second.m_total;
            name = score->first;
        }
    }
    return make_pair(name, max);
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
        ScoreMessage* msg = new ScoreMessage(Language::instance->get(TEXT_SCORE_MESSAGE)(m_playerOrder[i]),
                                m_boardPositions[i].m_position, 
                                Vector(0,1,0), 
                                m_scores[m_playerOrder[i]].m_total, 
                                m_boardPositions[i].m_alignement);
        m_messages->add2D(msg);
        m_scoreMessages.push_back(msg);

        //TODO: make universal
        float fontSize = 32;
        Vector correctorPos(0.0f, (fontSize + 2.0f) * m_boardPositions[i].m_nextDirectionY, 0.0f);
        
        ComboMessage* msg1 = new ComboMessage(Language::instance->get(TEXT_HITS),
                                m_boardPositions[i].m_position + correctorPos, 
                                Vector(0,1,0), 
                                m_scores[m_playerOrder[i]].m_combo, 
                                m_boardPositions[i].m_alignement);
        m_messages->add2D(msg1);
        m_selfComboMessages.push_back(msg1);

    }
}

int ScoreBoard::addTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += m_joinedCombo;
    return m_joinedCombo;
}

int ScoreBoard::addPoint(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += 1;
    return 1;
}

int ScoreBoard::addSelfTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += acc.m_combo;
    return acc.m_combo;
}

void ScoreBoard::incrementCombo(const string& name, const Vector& position)
{
    m_messages->add3D(new FlowingMessage(L"+1", position, Vector(0.0f,0.8f,0.8f), Font::Align_Center, 1.0f));
    m_joinedCombo++;
    Account& acc = m_scores.find(name)->second;
    acc.m_combo++;
}

void ScoreBoard::update()
{
    m_comboMessage->m_points = m_joinedCombo;
    for (size_t i = 0; i < m_playerOrder.size(); i++)
    {
        m_scoreMessages[i]->m_score = m_scores[m_playerOrder[i]].m_total;
        m_selfComboMessages[i]->m_points = m_scores[m_playerOrder[i]].m_combo;
    }
}
