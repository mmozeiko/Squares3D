#ifndef __MENU__H__
#define __MENU__H__

#include "common.h"
#include "state.h"
#include "system.h"
#include "vmath.h"

class Camera;
class Music;
class Game;
class Font;
struct Face;
class Texture;
class Menu;

typedef vector<wstring> Values;
class Value
{
public:
    Values        m_values;
    size_t        m_current;

    wstring getCurrent();
    void addAnother(const wstring& string);
    Value(const std::wstring& string);
    void activateNext();
};

class Entry
{
public: 
    wstring     m_string;
    Vector      m_position;
    const Font* m_font;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(const Vector& position, const wstring& stringIn, const Font* font);
    virtual void click() = 0;
    virtual wstring getString() = 0;
};

class OptionEntry : public Entry
{
    Value       m_value;
public: 
    OptionEntry(const Vector& position, const wstring& stringIn, const Value& value, const Font* font);
    wstring getString();
    void click();
};

class StartGameEntry : public Entry
{
public: 
    StartGameEntry(const Vector& position, const wstring& stringIn, Menu* menu, const Font* font);
    wstring getString();
    void click();

    Menu* m_menu;
};

//class MenuEntry : public Entry
//{
//public: 
//    MenuEntry(const Vector& position, const wstring& stringIn, Submenu* submenu, const Font* font);
//    wstring getString();
//    void click();
//
//    Menu* m_menu;
//};

typedef vector<Entry*> Entries;

class Submenu
{
public:

    Entries m_entries;
    Entry*  m_activeEntry;

    Submenu();
    ~Submenu();
    void addEntry(Entry* entry);
    void render() const;
    void control();
};

typedef vector<Submenu*> Submenus;

class Menu : public State
{
public:
    Menu();
    ~Menu();

    void control();
    void update(float delta);
    void updateStep(float delta);
    void prepare();
    void render() const;
    void loadMenu();
    State::Type progress() const;

    bool        m_goToGame;
    Music*      m_music;
    Camera*     m_camera;
    const Font* m_font;

private:
    Submenu*     m_currentSubmenu;
    Submenus     m_submenus;
    bool         m_quitGame;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};

#endif


//#meant to determine current menu
//class EntryIndex:
//  def __init__(self):  
//    self.value = 0
//    self.previousValue = 0
//  def set(self, val):
//    self.previousValue = self.value
//    self.value = val
//  def setPrevious(self):
//    temp = self.previousValue
//    self.previousValue = self.value
//    self.value = temp
//
//  def get(self):
//    return self.value
//
//class Entries(list):
//  def __init__(self):
//    self.coords = (0, 0)
//    self.baseColor = (0,0,1)
//    self.selectedColor = (1,0,0)
//    W, H = glfwGetWindowSize()
//    self.entryFontScale = gameglobals.fontScale * 1.5
//    self.entryLineSpacing = 3 * self.entryFontScale
//    
//    self.cursorPos = (W/2, H/2)
//    self.cursorAngle = 0.0
//    self.font_aa = GLFont(r"DATA\\FNT\\bold_aa.glf")
//    self.defaultColor = (0.0, 1.0, 1.0)
//    self.textureCursor = objects.loadTexture("DATA\\TEX\\cursor.png")
//
//  def addEntry(self, string, state):
//    #[text, active = False, state]
//    self.append([string, False, state])
//
//  def setSelectionIndex(self, selection, entryIndex):
//    if selection == 'Start Game':
//      entryIndex.set(1) #set the menu index to ingame menu
//    if selection == 'Restart Game':
//      entryIndex.set(1) #set the menu index to ingame menu
//    if selection == 'Exit to Main Menu':
//      entryIndex.set(0) #set the menu index to main menu
//    if selection == 'Rules of the game':
//      entryIndex.set(2) #set the menu index to Rules of the game menu
//
//  def applyInputs(self, entryIndex):
//    W, H = glfwGetWindowSize()
//
//    mx, my = glfwGetMousePos()
//    my = -my
//    
//    if (mx, my + H) != self.cursorPos:
//      if mx <= 0: mx = 0
//      if mx >= W: mx = W
//      if my >= 0: my = 0
//      if my <= -H: my = -H
//      self.cursorPos = (mx, my + H)
//      glfwSetMousePos(mx, -my)
//
//    #if mousebutton1 is pressed, enter selected menu
//    W, H = glfwGetWindowSize()
//    menuY = H / 2 + 50 * self.entryFontScale
//    for i in range(len(self)):
//      pos = i + 1
//      string = self[i][0]
//      #vertical positioning will work fine only with one height for all entries
//      x0 = W / 2 - self.font_aa.getSize(string)[0] * self.entryFontScale / 2
//      y0 = menuY - pos * self.font_aa.getSize(string)[1] * self.entryFontScale #+ self.entryLineSpacing
//      x1 = x0 + self.font_aa.getSize(string)[0] * self.entryFontScale
//      y1 = y0 + self.entryLineSpacing - self.font_aa.getSize(string)[1] * self.entryFontScale
//      if vectormath.isPointInSquare((self.cursorPos[0], 0, self.cursorPos[1]), (x0, y1), (x1, y0)):
//        self[i][1] = True
//        
//        if glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS\
//           and glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE:
//          sounds.playSound("Menu", (0,0,0))
//          selection = self[i][0]
//          gameglobals.gameState = self[i][2]
//          self.setSelectionIndex(selection, entryIndex)
//
//  def drawCursor(self):
//    x, y = self.cursorPos
//    self.font_aa.begin()
//    glBindTexture(GL_TEXTURE_2D, self.textureCursor)
//    glTranslatef(x, y, 0.0)
//    glRotatef(self.cursorAngle, 0.0, 0.0, 1.0)
//    self.cursorAngle += 0.5
//    glColor3f(1.0, 1.0, 1.0)
//    glBegin(GL_QUADS)
//    L = 10 * gameglobals.fontScale
//    glTexCoord2f(0.0, 0.0); glVertex2i(-L, -L)
//    glTexCoord2f(1.0, 0.0); glVertex2i( L, -L)
//    glTexCoord2f(1.0, 1.0); glVertex2i( L,  L)
//    glTexCoord2f(0.0, 1.0); glVertex2i(-L,  L)
//    glEnd()
//    self.font_aa.end()
//
//  def drawDeveloper(self):
//    W = glfwGetWindowSize()[0]
//    glColor4f(1.0, 1.0, 1.0, 0.9)
//    self.font_aa.begin()
//    dScale = gameglobals.fontScale / 2
//    x = W - (self.font_aa.getSize(gameinfo.developer)[0] + 5) * dScale
//    y = (self.font_aa.getSize(gameinfo.developer)[1] + 5) * dScale
//    self.font_aa.renderOutlined(gameinfo.developer, x, y, (0,0,0), dScale)
//    self.font_aa.end()
//
//  def drawTitle(self):
//    W, H = glfwGetWindowSize()
//    title = ["3D game", "Squares"]
//    glColor4f(0.0, 0.0, 1.0, 0.9)
//    self.font_aa.begin()
//
//    #unscaled font size is 14, i believe
//    titleScale = gameglobals.fontScale * 2
//    y = H - H / 8
//    for row in title:
//      x = W / 2 - self.font_aa.getSize(row)[0] * titleScale / 2
//      self.font_aa.renderOutlined(row, x, y, (0,0,0), titleScale)
//      y -= self.font_aa.getSize(row)[1] * titleScale / 2
//      titleScale = gameglobals.fontScale * 4
//    self.font_aa.end()        
//
//  def draw(self):
//    W, H = glfwGetWindowSize()
//    menuY = H / 2 + 50 * self.entryFontScale
//    for i in range(len(self)):
//      pos = i + 1
//      string = self[i][0]
//      #vertical positioning will work fine only with one height for all entries
//      x = W / 2 - self.font_aa.getSize(string)[0] * self.entryFontScale / 2
//      y = menuY - pos * self.font_aa.getSize(string)[1] * self.entryFontScale + self.entryLineSpacing
//      if self[i][1] == True: #if active
//        color = [1,0,0]
//      else:
//        color = self.defaultColor
//      glColor3f(color[0], color[1], color[2])
//      self.font_aa.begin()
//      self.font_aa.render(string, x, y, self.entryFontScale)
//      self.font_aa.end()
//      self[i][1] = False
//    self.resetColors()
//    #render cursor
//    self.drawDeveloper()
//    
//  def resetColors(self):
//    for entry in self:
//      entry[1] = self.defaultColor
//
//def renderLongText(font, text, scale, x, y):
//  height = (font.getSize("Height")[1] / 2 + 5) * scale
//  font.begin()
//  for line in text.split('\n'):
//    font.renderOutlined(line, x, y, (0, 0, 0), scale)
//    y -= height        
//  font.end()
//
//
//class MainEntries(Entries):
//#this is main menu 
//  def __init__(self):
//    Entries.__init__(self)
//
//    #format:
//    #string, according state
//
//    #menu = 0
//    #start game anew = 1
//    #resume game = 2
//    #quit game = 3
//
//    self.addEntry('Start Game', 2)
//    self.addEntry('Rules of the game', 0)
//    self.addEntry('About', 0)
//    self.addEntry('Exit Game', 3)
//    
//  def setSelectionIndex(self, selection, entryIndex):
//    if selection == 'Start Game':
//      entryIndex.set(1) #set the menu index to ingame menu
//    if selection == 'Rules of the game':
//      entryIndex.set(2) #set the menu index to Rules of the game menu
//    if selection == 'About':
//      entryIndex.set(3) #set the menu index to About menu
//
//  def draw(self):
//    Entries.draw(self)
//    #render game title
//    self.drawTitle()
//
//class InGameEntries(Entries):
//#this is ingame menu 
//  def __init__(self):
//    Entries.__init__(self)
//    self.addEntry('Resume', 1)
//    self.addEntry('Restart Game', 2)
//    self.addEntry('Rules of the game', 0)
//    self.addEntry('Exit to Main Menu', 0)
//    
//  def setSelectionIndex(self, selection, entryIndex):
//    if selection == 'Restart Game':
//      entryIndex.set(1) #set the menu index to ingame menu
//    if selection == 'Exit to Main Menu':
//      entryIndex.set(0) #set the menu index to main menu
//    if selection == 'Rules of the game':
//      entryIndex.set(2) #set the menu index to Rules of the game menu
//
//  def draw(self):
//    Entries.draw(self)
//    #render game title
//    self.drawTitle()
//
//class RulesEntries(Entries):
//#this is ingame menu 
//  def __init__(self):
//    Entries.__init__(self)
//    for i in range(0, 4): self.addEntry(' ', 0) #stuff the dummies to push down "Back"
//    self.addEntry('Back', 0)
//    
//  def setSelectionIndex(self, selection, entryIndex):
//    if selection == 'Back':
//      entryIndex.setPrevious() #set the menu index to previous menu
//
//  def draw(self):
//    Entries.draw(self)
//    #draw rules text
//    H = glfwGetWindowSize()[1]
//    fontScale = gameglobals.fontScale / 1.35
//    x = 5 * fontScale
//    y = H - x
//    glColor3f(1,1,1)
//    renderLongText(self.font_aa, gameinfo.rules, fontScale, x, y)
//
//class AboutEntries(RulesEntries):
//  def draw(self):
//    Entries.draw(self)
//    #draw about info
//    H = glfwGetWindowSize()[1]
//    fontScale = gameglobals.fontScale / 1.35
//    glColor3f(1,1,1)
//    sep = "\n \n \n"
//    bigText = gameinfo.info + sep + gameinfo.controls + sep + gameinfo.about + sep + gameinfo.credits
//    fontHeight = self.font_aa.getSize("Height")[1] * fontScale
//    x = 5 * fontScale
//    y = H - x
//    renderLongText(self.font_aa, bigText, fontScale, x, y)
//    #draw version
//    self.font_aa.begin()
//    self.font_aa.renderOutlined(gameinfo.version, 5 * fontScale, fontHeight, (0,0,0), fontScale)
//    self.font_aa.end()
//    
//
//  def drawDeveloper(self):
//    W = glfwGetWindowSize()[0]
//    glColor4f(1.0, 1.0, 1.0, 0.9)
//    self.font_aa.begin()
//    dScale = gameglobals.fontScale / 2
//    x = W - (self.font_aa.getSize(gameinfo.developer)[0] + 5) * dScale
//    y = (self.font_aa.getSize(gameinfo.developer)[1] + 5) * dScale
//    self.font_aa.renderOutlined(gameinfo.developer, x, y, (0,0,0), dScale)
//    self.font_aa.end()
//
//  
//class GameMenu(ode.World):
//  def __init__(self): 
//    self.objectList = [] 
//    self.performance = 0
//    ode.World.__init__(self)
//    self.setGravity((0.0, -9.81/2, 0.0))
//    self.space = ode.Space()
//    self.contactgroup = ode.JointGroup()
//    self.time0 = 0
//    #here are menus: mainmenu, ingame
//    self.entryList = [MainEntries(), InGameEntries(), RulesEntries(), AboutEntries()]
//    self.entryIndex = EntryIndex()
//    self.font_aa = GLFont(r"DATA\\FNT\\bold_aa.glf")
//    self.camera = Camera(0.0, 24.0, 0.0, -30.0, 0.0) 
//    
//  def launchClock(self):
//    self.time0 = CLOCK()
//
//  def applyInputs(self):
//    glfwEnable(GLFW_STICKY_MOUSE_BUTTONS)
//    #apply menu inputs
//    self.entryList[self.entryIndex.get()].applyInputs(self.entryIndex)
//    
//  def draw(self): 
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
//
//    glMatrixMode(GL_MODELVIEW)
//    glLoadIdentity()
//
//    self.camera.draw()    
//
//    glLightfv(GL_LIGHT0, GL_POSITION, (0.0, 10.0, 0.0, 1.0))
//  
//    for part in self.objectList:
//      glPushMatrix()
//      part.draw()
//      glPopMatrix()
//
//    #draw active menu
//    self.entryList[self.entryIndex.get()].draw()
//    self.entryList[self.entryIndex.get()].drawCursor()
//
//  def addObject(self, object):
//    self.objectList.append(object)
//
//  def idle(self): 
//    gameMenu = self
//    newTime = CLOCK()
//    deltaTime = newTime - gameMenu.time0
//    
//    if deltaTime >= 1.0 / 60: #maxfps = 60
//      gameMenu.performance = deltaTime
//      gameMenu.applyInputs()
//
//      gameMenu.space.collide((gameMenu, gameMenu.contactgroup), near_callback)
//
//      if deltaTime < 0.015: DT = 0.015
//      else: DT = deltaTime
//      
//      gameMenu.step(DT)
//      gameMenu.contactgroup.empty()    
//     
//      gameMenu.draw()
//      glfwSwapBuffers()
//      gameMenu.time0 = newTime
