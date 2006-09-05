#include "referee.h"
#include "player.h"
#include "body.h"
#include "geometry.h"

Referee::Referee(): 
    m_gameOver(false)
{
    initEvents();
}

void Referee::initEvents()
{
    m_lastFieldOwner    = NULL;
    m_lastTouchedObject = NULL;
    m_lastTouchedPlayer = NULL;
}

void Referee::registerPlayer(const string& name, Player* player)
{
    m_players[player->m_body] = std::make_pair(name, player);
}

void Referee::process(Body* body1, Body* body2)
{
    if (!m_gameOver)
    {
        if (body1 == m_ball)
        {
            registerBallEvent(body1, body2);
        }
        if (foundInMap(m_players, body1))
        {
            registerPlayerEvent(body1, body2);
        }
    }
}

void Referee::registerPlayerEvent(Body* player, Body* otherBody)
{
    if (otherBody == m_ground)
    {
        processPlayerGround(player);
    }
}

void Referee::registerBallEvent(Body* ball, Body* otherBody)
{
    if (otherBody == m_ground)
    {
        processBallGround(ball, otherBody);
    }
    else if (foundInMap(m_players, otherBody))
    {
        processBallPlayer(ball, otherBody);
    }
}

void Referee::processPlayerGround(Body* player)
{
    Vector playerPos = player->getPosition();
    string currentPlayerName = m_players[player].first;
    Player* currentPlayer = m_players[player].second;
    if (!isPointInRectangle(playerPos, currentPlayer->m_lowerLeft, currentPlayer->m_upperRight)
        && isPointInRectangle(playerPos, 
                              Vector(- FIELDLENGTH, 0 , - FIELDLENGTH),
                              Vector(FIELDLENGTH, 0 , FIELDLENGTH)))
    {
        clog << "pleijeris " + currentPlayerName + " ir iekaapis cita pleijera laukumaa!!.." << endl;
    }
}

void Referee::processBallGround(Body* ball, Body* otherBody)
{
    Vector ballPos(ball->getPosition());

    //out = False

    if (!isPointInRectangle(ballPos, 
                            Vector(- FIELDLENGTH, 0.0f, - FIELDLENGTH), 
                            Vector(FIELDLENGTH, 0.0f, FIELDLENGTH))) 
    {
        //critical event
        //BALL HAS HIT THE FIELD OUTSIDE
        //and we handle score here

        if (m_lastTouchedObject != NULL) //if last touched was not middle line
        {    
            if (foundInMap(m_players, m_lastTouchedObject))
            {
                clog << m_players[m_lastTouchedObject].first + " izsit bumbu laukaa" << endl;
            //player has kicked the ball out
//          pts = self.scoreBoard.addSelfPoints(lastTouchedObject)
//          resetCoords = self.getBallCoords(lastTouchedObject)
//          self.faultMsgList.append(['Out from ' + lastTouchedObject + '!', str(pts), 0])
//
//          out = lastTouchedObject
//          
//          getsPoints = lastTouchedObject
            }

            else if (m_lastFieldOwner != NULL) //if ground was touched in one of the players field last add points to owner
            {
                string owner = m_players[m_lastFieldOwner].first;
                clog << "bumba izripo no " + owner + " laukuma!" << endl;
    //          if self.events['lastTouchedPlayer']:
    //            #case when noone touched the ball after throwing minus
    //            lastPlayer = self.events['lastTouchedPlayer'].name
    //            if owner == lastPlayer:
    //              pts = self.scoreBoard.addSelfPoints(owner)
    //            else:
    //              pts = self.scoreBoard.addTotalPoints(owner)
    //          else:
    //            pts = self.scoreBoard.addPoint(owner)
    //
    //          out = owner
    //
    //          self.faultMsgList.append(['Out from ' + owner + '`s field!', str(pts), 0])
    //          resetCoords = self.getBallCoords(owner)
    //          getsPoints = owner
    //        if self.scoreBoard[getsPoints][0] >= self.matchPoints:
    //          self.setGameOver()
    //          return
            }
        }
        else
        {
            clog << "bumba izripo no vidliinijas, nuubi iet uz skolu" << endl;
//        self.faultMsgList.append(['Ball out from middle line!', '', 0])
//        resetCoords = (0, 10, 0)
        }
//
//      if out=="Player":
//        pl = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
//        #pl2 = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
//        #while pl2==pl:
//        #  pl2 = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
//
//        sounds.playTauntMsg(pl, "GreatShot", pl.body.getPosition())
//        self.addTaunt(pl, "Great shot!")
//
//        #    sounds.playTauntMsg(pl2, "Yes", pl.body.getPosition())
//        #    self.addTaunt(pl2, "Yes!")
//
//##      print self.scoreBoard
//
//      #finishing handling critical branch
//      self.scoreBoard.resetJoinedCombo()
  
        initEvents();
        m_ball->setTransform(Vector(0,2,0), Vector(0,0,0));
        NewtonBodySetVelocity(m_ball->m_newtonBody, Vector().v);

//      self.resetPlayers()
//      if out!=False and out!="Player":
//        pl = self.players[out]
//        sounds.playTauntMsg(pl, "MoveIn", pl.body.getPosition())
//        self.addTaunt(pl, "Move in!")
//
//
    }
    else
    {
      //BALL HAS HIT THE FIELD INSIDE
      //save the touched field
      for each_const(BodyToPlayerDataMap, m_players, player)
      {
          if (isPointInRectangle(ballPos, 
                                 player->second.second->m_lowerLeft, 
                                 player->second.second->m_upperRight))
          {
             m_lastFieldOwner =    player->first;
             m_lastTouchedObject = m_ground;
             break;
          }
          m_lastTouchedObject = NULL; //if has hit the middle line
      }
        
    }
}

void Referee::processBallPlayer(Body* ball, Body* player)
{
    //ball + player

//    sounds.playSound("BallPlayer", ball.geom.getPosition())
    if (m_lastTouchedObject == NULL)
    {
//      self.scoreBoard.resetJoinedCombo() #clear combo
//      self.scoreBoard.incrementCombo(player, self.hitMsgList) #(+1)
    }
    else //picked from ground or player
    {
        if (m_lastTouchedObject == m_ground) //picked from ground inside
        {
//          self.scoreBoard.resetJoinedCombo()
            string playerName = m_players[player].first;
//          self.scoreBoard.incrementCombo(player, self.hitMsgList)
            if ((m_lastFieldOwner == player)
                && (m_lastTouchedPlayer == player)
                && (isPointInRectangle(m_ball->getPosition(), 
                                       m_players[player].second->m_lowerLeft, 
                                       m_players[player].second->m_upperRight)))
            {
                //critical event. double-touched -> fault

                clog << playerName + " tuu taches!!" << endl;
    //          if playerName=="Player":
    //            pl = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
    //            #pl2 = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
    //            #while pl2==pl:
    //            #  pl2 = self.players[["Player_Red", "Player_Green", "Player_Yellow"][randint(0,2)]]
    //
    //            sounds.playTauntMsg(pl, "AreYouCrazy", pl.body.getPosition())
    //            self.addTaunt(pl, "Are you crazy?")
    //
    //            #sounds.playTauntMsg(pl2, "No", pl.body.getPosition())
    //            #self.addTaunt(pl2, "No!")
    //
    //          resetCoords = self.getBallCoords(playerName)
    //          self.scoreBoard.addSelfPoints(playerName)
    //          self.faultMsgList.append([playerName + ' touched twice!', '1', 0])
    //          if self.scoreBoard[playerName][0] >= self.matchPoints:
    //            self.setGameOver()
    //            return
                initEvents();
    //          self.resetBall(resetCoords)
    //          self.resetPlayers()
                return ;
            }
        }
        else //picked from player
        {
//        if self.events['lastTouchedPlayer'] != player: #reset own combo, when picked from other
//          self.scoreBoard.resetOwnCombo(player.name)
//        self.scoreBoard.incrementCombo(player, self.hitMsgList)
        }
    }
//        
    m_lastTouchedObject = player;
    m_lastTouchedPlayer = player;
}

//class Coach:
//  def __init__(self):
//    self.state = 1
//    self.afterCollideTriggerBox = False
//    self.beforeCollideTriggerBox = False
//    self.initEvents()
//    self.matchPoints = 21
//    
//    self.ball = None
//    self.gameOver = False
//
//    #faults list will contain msg text, points, and delta up px for text
//    self.faultMsgList = []
//    #hits list will contain combo, delta up px and coords of the collision (X HITS!)
//    self.hitMsgList = []
//
//    self.taunts = []
//
//    self.players = {}
//    self.scoreBoard = ScoreBoard(self.players)
//    self.objectList = []
//    self.font_aa = GLFont(r"DATA\\FNT\\bold_aa.glf")
//      
//  def addTaunt(self, player, msg):
//    pos = [player.body.getPosition()[0], player.body.getPosition()[1], player.body.getPosition()[2]]
//    self.taunts.append([msg,
//                        1.0,
//                        pos, player.color])
//
//  def restart(self):
//    self.gameOver = False
//    self.scoreBoard.reset()
//    self.resetBall()
//    self.resetPlayers()
//
//  def setGameOver(self):
//    sounds.playSound("GameOver", (0,0,0))
//    self.gameOver = True
//
//  def getLoserNameColor(self):
//    for name, object in self.players.items():
//      if self.scoreBoard[name][0] >= self.matchPoints:
//        return name, object.color
//    
//  def initEvents(self):
//    self.events = {'lastFieldOwner':None,
//                   'lastTouchedObject':None,
//                   'lastTouchedPlayer':None}
//
//  def resetBall(self, resetCoords = (0.0, 10.0, 0.0)):
//    if resetCoords == (0.0, 10.0, 0.0): #a little bit misadjust centered coords
//      pairs = ((-0.01, 0.0), (-0.01, -0.01), (0.0, -0.01), (0.01, -0.01), (0.01, 0.0), (0.01, 0.01), (0.0, 0.01))
//      newXZ = pairs[randint(0, 6)]
//      resetCoords = (newXZ[0], 10.0, newXZ[1])
//      
//    self.ball.geom.reset = True
//    self.ball.geom.resetCoords = resetCoords
//
//  def resetPlayers(self):
//    for player in self.players.values():
//      player.setInitPosition()
//      player.setAngle()
//
//  def getBallCoords(self, playerName):
//    player = self.players[playerName]
//    littleBit = ((player.min[0] + player.max[0]) / 5, (player.min[1] + player.max[1]) / 5)
//    ballPos = ((player.min[0] + player.max[0]) / 2 - littleBit[0],
//                  3.3, (player.min[1] + player.max[1]) / 2 - littleBit[1])
//    return ballPos
//
//  def checkIsBallInSquare(self, ball, min, max):
//    min0 = min[0]
//    min1 = min[1]
//    max0 = max[0]
//    max1 = max[1]
//    #0.5 is for the middle line 
//    if min0 == 0: min0 = max0 / 10 * 0.5
//    if min1 == 0: min1 = max1 / 10 * 0.5
//    if max0 == 0: max0 = min0 / 10 * 0.5
//    if max1 == 0: max1 = min1 / 10 * 0.5
//    return vectormath.isPointInSquare(ball, (min0, min1), (max0, max1))
//
//    
////  def register(self, player):
////    name = player.geom.name
////    self.players[name] = player
////    #see scoreBoard.reset
////    self.scoreBoard[name] = [0, 0]
//
//  def msgFlowFunction(self, x, px):
//    #this method is used to make the fault messages and "HITS" msg flow up and dissapear
//    return 1.0 - (x/px) * (x/px)* (x/px)* (x/px)
//
//  def draw(self, performance):
//    W, H = glfwGetWindowSize()
//
//    scale = gameglobals.fontScale
//    
//    nameScoreList = []
//
//    #draw players scores
//    player = self.players['Player']
//    text = 'Player: ' + str(self.scoreBoard['Player'][0])
//    x = 10.0
//    y = self.font_aa.getSize(text)[1] * scale
//    nameScoreList.append([text, x, y, player.color])
//
//    playerName = 'Player_Red'
//    player = self.players[playerName]
//    text = playerName + ': ' + str(self.scoreBoard[playerName][0])
//    x = 10.0
//    y = H
//    nameScoreList.append([text, x, y, player.color])
//
//    playerName = 'Player_Green'    
//    player = self.players[playerName]
//    text = playerName + ': ' + str(self.scoreBoard[playerName][0])
//    x = W - self.font_aa.getSize(text)[0] * scale - 10
//    y = H
//    nameScoreList.append([text, x, y, player.color])
//
//    playerName = 'Player_Yellow'
//    player = self.players[playerName]
//    text = playerName + ': ' + str(self.scoreBoard[playerName][0])
//    x = W - self.font_aa.getSize(text)[0] * scale - 10
//    y = self.font_aa.getSize(text)[1] * scale
//    nameScoreList.append([text, x, y, player.color])
//    
//    self.font_aa.begin()
//    for nameScore in nameScoreList:
//      glColor3f(nameScore[3][0], nameScore[3][1], nameScore[3][2])      
//      self.font_aa.renderShadowed(nameScore[0], nameScore[1], nameScore[2], (0,0,0), scale)
//    self.font_aa.end()
//
//    #draw "hits" messages
//    px = 50
//    for hit in self.hitMsgList:
//      if hit[1] < px:
//        addend = self.msgFlowFunction(hit[1], px)
//        hitText = ' HITS!'
//        if hit[0] < 2:
//          hitText = ' HIT!'
//        text = '+' + str(hit[0]) + hitText
//        pCoords = hit[2]
//
//        x, y, _ = gluProject(pCoords[0], pCoords[1], pCoords[2])
//        y = y + hit[1]
//      
//        glColor4f(hit[3][0], hit[3][1], hit[3][2], addend)
//        self.font_aa.begin()
//        self.font_aa.renderOutlined(text, x, y, (0,0,0), scale)
//        self.font_aa.end()
//        hit[1] += 200 * (1.2 - addend) * performance
//      else:
//        self.hitMsgList = self.hitMsgList[1:]
//
//    #draw the faults messages        
//    px = 80
//    for fault in self.faultMsgList:
//      if fault[2] < px:
//        addend = self.msgFlowFunction(fault[2], px)
//        text = fault[0]
//        pts = fault[1]
//        x, y = W / 2 - self.font_aa.getSize(text)[0] * scale / 2, H - 50 * scale
//        y = y + fault[2]
//      
//        glColor4f(1.0, 0.8, 0.0, addend)
//        self.font_aa.begin()
//        self.font_aa.renderShadowed(text, x, y, (0,0,0), scale)
//        if pts:
//          glColor4f(0.5, 0.8, 0.1, addend)
//          pts = '+' + pts
//          x = W / 2 - self.font_aa.getSize(pts)[0] * scale / 2
//          y -= self.font_aa.getSize(pts)[1] * scale
//          self.font_aa.renderShadowed(pts, x, y, (0,0,0), scale)
//        self.font_aa.end()
//        fault[2] += 200 * (1.2 - addend) * performance
//      else:
//        self.faultMsgList = self.faultMsgList[1:]
//        
//    #draw the taunts
//    px = 100
//    for taunt in self.taunts:
//      if taunt[1] < px:
//        addend = self.msgFlowFunction(taunt[1], px)
//        text = taunt[0]
//
//        pCoords = taunt[2]
//        x, y, _ = gluProject(pCoords[0], pCoords[1], pCoords[2])
//        y = y + taunt[1]
//        x = x - self.font_aa.getSize(text)[0] * scale / 2
//
//        c = taunt[3] #players.colorNamesColors[taunt[2]]
//      
//        glColor4f(c[0], c[1], c[2], addend)
//        self.font_aa.begin()
//        self.font_aa.renderShadowed(text, x, y, (0,0,0), scale)
//        self.font_aa.end()
//        taunt[1] += 200 * (1.2 - addend) * performance
//      else:
//        self.taunts= self.taunts[1:]
//
//
//    #draw current total combo
//    total = self.scoreBoard.getTotalPoints()
//    if total > 1:
//      comboScale = scale * 1.25
//      totalText = 'Combo ' + str(total) + '!'
//      x = W / 2 - self.font_aa.getSize(totalText)[0] * comboScale / 2
//      y = 2 * self.font_aa.getSize(totalText)[1] * comboScale
//    
//      glColor4f(0.0, 1.0, 1.0, 1)
//      self.font_aa.begin()
//      self.font_aa.renderShadowed(totalText, x, y, (0,0,0), comboScale)
//      self.font_aa.end()
//      
//    #draw game over message
//    if self.gameOver == True:
//      loserMsg, loserColor = self.getLoserNameColor()
//      loserMsg += ' lost!'
//      msg = 'Game Over'
//      loserScale = scale * 1.25
//      gOverScale = scale * 2.5
//      
//      x0 = W / 2 - self.font_aa.getSize(loserMsg)[0] * loserScale / 2
//      y0 = H / 2 + H / 4 -  self.font_aa.getSize(loserMsg)[1] * loserScale / 2
//
//      x1 = W / 2 - self.font_aa.getSize(msg)[0] * gOverScale / 2
//      y1 = H / 2 -  self.font_aa.getSize(msg)[1] * gOverScale / 2
//    
//      self.font_aa.begin()
//      glColor4f(loserColor[0], loserColor[1], loserColor[2], 0.9)
//      self.font_aa.render(loserMsg, x0, y0, loserScale)
//      glColor4f(1.0, 0.0, 0.0, 0.9)
//      self.font_aa.render(msg, x1, y1, gOverScale)
//      self.font_aa.end()

  

void Referee::manageGame()
//This is the main logic function of the coach
//Coach has 3 states to keep in mind when registering event
//States are needed to avoid registering ball+player event when
//the ball is standing on player not bouncing
//This is the place where TriggerBox is used
{
    //if (!m_gameOver):
    //{
//      objectNames = [object.name for object in self.objectList]
//      beforeCollideTriggerBox = self.afterCollideTriggerBox #initvalue=False
        //m_afterCollideTriggerBox = true; //('Trigger' in objectNames)# True if ball and TriggerBox collide else False
        //afterCollidePlayer = ('Player' in ''.join(objectNames)) # True if ball and Player collide else False
//      if 'Ground' in objectNames:
//        self.registerGroundEvent()
//     
//      if self.state==1 and beforeCollideTriggerBox==False and self.afterCollideTriggerBox==True:
//        self.state=2
//      if self.state==2 and afterCollide Player==True:
//        self.registerPlayerEvent()
//        self.state=3
//      elif self.state==2 and beforeCollideTriggerBox==True and self.afterCollideTriggerBox==False:
//        self.state=1
//      elif self.state==3 and self.afterCollideTriggerBox==False:
//        self.state=1
}

//from random import randint
//from glfont.glfont import GLFont
//from pyglfw.glfw import glfwGetWindowSize
//from OpenGL.GL import *
//from OpenGL.GLU import gluProject
//from sound import sounds
//import vectormath
//import gameglobals
//
//
//class ScoreBoard(dict):
//  def __init__(self, players):
//    self.players = players
//    self.joinedCombo = 0
//
//  def getTotalPoints(self):
//    return self.joinedCombo
//
//  def addTotalPoints(self, playerName):
//    pts = self.getTotalPoints()
//    self[playerName][0] += pts #update player score account
//    self.resetJoinedCombo()
//    return pts
//
//  def addSelfPoints(self, playerName):
//    playerAccount = self[playerName]
//    pts = playerAccount[1]
//    playerAccount[0] += pts #update player score account by earned pts
//    self.resetJoinedCombo()
//    return pts
//
//  def incrementCombo(self, player, hitMsgList):
//    playerName = player.name
//    self[playerName][1] += 1 #update own combo
//    self.joinedCombo += 1 #update joined combo
//    #renew floating coords
//    hitMsgList.append([self[playerName][1],
//                       1.0,
//                       player.getPosition(),
//                       self.players[playerName].color])
//
//  def addPoint(self, playerName):
//    self[playerName][0] += 1
//    return 1
//
//  def resetJoinedCombo(self):
//    for val in self.values(): #reset player combos
//      val[1] = 0 #null them
//    self.joinedCombo = 0 #reset joined as well
//    
//  def reset(self):
//    for playerName in self.players.keys():
//      # (player`s total score (minuses), current combo points
//      self[playerName] = [0, 0]
//    self.joinedCombo = 0
//
//  def resetOwnCombo(self, playerName):
//    self[playerName][1] = 0
//
//
