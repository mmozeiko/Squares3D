#include "referee.h"
#include "scoreboard.h"


Account::Account() : 
    m_total(0),
    m_combo(0)
{
}

ScoreBoard::ScoreBoard()
{
    reset();
}

void ScoreBoard::registerPlayer(const string& name)
{
    m_scores[name] = Account();
}

void ScoreBoard::resetCombo()
{
    m_joinedCombo = 0;
    for each_(Scores, m_scores, iter)
    {
        Account& acc = iter->second;
        acc.m_combo = 0;
    }
}

void ScoreBoard::reset()
{
    resetCombo();
    for each_(Scores, m_scores, iter)
    {
        iter->second = Account();
    }
}

void ScoreBoard::addTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += m_joinedCombo;
}

void ScoreBoard::addPoint(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += 1;
}

void ScoreBoard::addSelfTotalPoints(const string& name)
{
    Account& acc = m_scores.find(name)->second;
    acc.m_total += acc.m_combo;
}

void ScoreBoard::incrementCombo(const string& name)
{
    m_joinedCombo++;
    Account& acc = m_scores.find(name)->second;
    acc.m_combo++;
}

void ScoreBoard::render()
{
	

}
