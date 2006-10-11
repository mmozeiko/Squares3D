#include "world.h"
#include "player.h"
#include "camera.h"
#include "video.h"
#include "audio.h"
#include "player_local.h"
#include "player_ai.h"
#include "input.h"
#include "level.h"
#include "music.h"
#include "sound.h"
#include "file.h"
#include "skybox.h"
#include "properties.h"
#include "referee.h"
#include "ball.h"
#include "messages.h"
#include "message.h"
#include "language.h"
#include "scoreboard.h"
#include "config.h"
#include "framebuffer.h"
#include "grass.h"
#include "network.h"
#include "profile.h"
#include "colors.h"

static const float OBJECT_BRIGHTNESS_1 = 0.25f; // shadowed
static const float OBJECT_BRIGHTNESS_2 = 0.4f; // lit
static const float GRASS_BRIGHTNESS_1 = 0.6f; // shadowed
static const float GRASS_BRIGHTNESS_2 = 0.7f; // lit

static const Vector playerPositions[4] = {Vector(- FIELDLENGTH / 2, 1.5f, - FIELDLENGTH / 2),
                                          Vector(- FIELDLENGTH / 2, 1.5f,   FIELDLENGTH / 2),
                                          Vector(  FIELDLENGTH / 2, 1.5f,   FIELDLENGTH / 2),
                                          Vector(  FIELDLENGTH / 2, 1.5f, - FIELDLENGTH / 2)
                                          };

template <class World> World* System<World>::instance = NULL;

State::Type World::progress()
{
    int key;
    do
    {
        key = Input::instance->popKey();
        if (key == GLFW_KEY_ESC)
        {
            if (m_freeze)
            {
                m_freeze = false;
                m_messages->remove(escMessage);
                return State::Current;
            }
            else
            {
                m_freeze = true;
                float resY = static_cast<float>(Video::instance->getResolution().second);
                escMessage = new Message(
                                Language::instance->get(TEXT_ESC_MESSAGE), 
                                Vector(static_cast<float>(Video::instance->getResolution().first / 2), 
                                       resY / 2 + resY / 4, 
                                       0.0f), 
                                Blue,
                                Font::Align_Center);
                m_messages->add2D(escMessage);
            }
        }

        if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)) 
        {
            State::Type returnState;
            if (m_referee->m_gameOver)
            {
                if (m_referee->getLoserName() != m_userProfile->m_name) 
                {
                    //if the player didn`t lose, we unlock next difficulty if not unlocked already
                    switch (m_difficulty)
                    {
                    case 0: returnState = State::MenuNormal;
                            break;
                    case 1: returnState = State::MenuHard;
                            break;
                    case 2: //completed on hard unlock maybe a special level here..
                            //for now just return MenuEasy (or could as well be Normal or Hard)
                            // - still nothing to unlock anymore
                            returnState = State::MenuEasy;
                            break;
                    default: //something is a miss
                            throw ("difficulty > 2");
                    }
                }
                else
                {
                    //here we don`t change the unlockable
                    if (m_freeze)
                    {
                        //user wants to leave to menu and doesn`t want to retry - return State::MenuEasy
                        returnState = State::MenuEasy;
                    }
                    else
                    {
                        //if none from above - we return State::m_current to retry (reload the same level)
                        returnState = m_current;
                    }
                }
                Input::instance->endKeyBuffer();
            }
            else
            {
                if (m_freeze)
                {
                    //user just wants to leave to menu in the middle of game - return State::MenuEasy
                    //it DOESN`T change the unlockable level
                    returnState = State::MenuEasy;
                    Input::instance->endKeyBuffer();
                }
                else
                {
                    //don`t change anything
                    returnState = State::Current;
                }
            }
            return returnState;            
        }
    }
    while (key != -1);

    return State::Current;
}

World::World(const Profile* userProfile, int difficulty) : 
    m_music(NULL),
    m_camera(NULL),
    m_skybox(NULL),
    m_grass(NULL),
    m_newtonWorld(NULL),
    m_level(NULL),
    m_ball(NULL),
    m_referee(NULL),
    m_messages(NULL),
    m_scoreBoard(NULL),
    m_freeze(false),
    m_userProfile(userProfile),
    escMessage(NULL),
    m_framebuffer(NULL),
    m_difficulty(difficulty)
{
    setInstance(this); // MUST go first

    m_framebuffer = new FrameBuffer();
    setupShadowStuff();
    setLight(Vector(-15.0f, 35.0f, 38.0f));

    //m_camera = new Camera(Vector(0.0f, 25.0f, 0.0f), 90.0f, 0.0f);
    m_camera = new Camera(Vector(0.0f, 9.0f, 14.0f), 30.0f, 0.0f);
    m_skybox = new SkyBox();
    m_messages = new Messages();
    m_scoreBoard = new ScoreBoard(m_messages);

    m_newtonWorld = NewtonCreate(NULL, NULL);

    // enable some Newton optimization
    NewtonSetSolverModel(m_newtonWorld, 1);
    
    m_music = Audio::instance->loadMusic("music.ogg");
    //m_music->play();

    m_level = new Level();
}

void World::init()
{
    StringSet tmp;
    m_level->load("level.xml", tmp);
    m_grass = new Grass(m_level);

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_referee = new Referee(m_messages, m_scoreBoard);
    m_referee->m_field = m_level->getBody("field"); //referee now can recognize game field
    m_referee->m_ground = m_level->getBody("level"); //referee now can recognize ground outside

    m_ball = new Ball(m_level->getBody("football"), m_level->m_collisions["level"]);
    m_referee->registerBall(m_ball);

    Player* human = new LocalPlayer(m_userProfile, m_level);
    m_referee->m_humanPlayer = human;
    m_localPlayers.push_back(human);

    addShuffledCpuProfiles(&m_level->m_cpuProfiles[m_difficulty], 3);
    
    for (size_t i = 0; i < m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
        m_ball->addBodyToFilter(m_localPlayers[i]->m_body);
    }

    m_referee->registerPlayers(m_localPlayers);

    Network* net = Network::instance;
    for each_const(BodiesMap, m_level->m_bodies, iter)
    {
        if (iter->second->getMass() > 0.0f)
        {
            net->add(iter->second);
        }
    }


    m_scoreBoard->reset();
    Input::instance->startKeyBuffer();
}

World::~World()
{
    Input::instance->endKeyBuffer();

    killShadowStuff();
    delete m_framebuffer;
    if (m_grass != NULL)
    {
        delete m_grass;
    }

    m_music->stop();
    Audio::instance->unloadMusic(m_music);

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        delete *iter;
    }

    delete m_ball;
    delete m_referee;
    delete m_level;

    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

    delete m_scoreBoard;
    delete m_messages;
    delete m_skybox;
    delete m_camera;
}

void World::control()
{
    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE)
    {
        // only camera and local players
        m_camera->control();
        m_localPlayers.front()->control();
    }
    // other objects go after this

    for (size_t i=1; i<m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->control();
    }
}

void World::updateStep(float delta)
{
    // updateStep is called more than one time in frame

    if (!m_freeze)
    {
        m_ball->triggerBegin();
        NewtonUpdate(m_newtonWorld, delta);
        m_ball->triggerEnd();

        for (size_t i=0; i<m_localPlayers.size(); i++)
        {
            if (m_localPlayers[i]->getPosition().y < -2.0f)
            {
                m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
            }
        }

        if (m_ball->getPosition().y < -2.0f)
        {
            m_referee->registerBallEvent(m_ball->m_body, m_level->getBody("level")); 
            m_referee->resetBall();
            m_referee->m_mustResetBall = false;
        }

    }
}

void World::update(float delta)
{
    // update is called one time in frame

    m_camera->update(delta);
    m_scoreBoard->update();
    m_referee->update();
    m_messages->update(delta);
    m_grass->update(delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render() const
{
    int shadow_type = Config::instance->m_video.shadow_type;
    if (shadow_type == 0)
    {
        glLightfv(GL_LIGHT1, GL_POSITION, m_lightPosition.v);
        glLightfv(GL_LIGHT1, GL_AMBIENT, (OBJECT_BRIGHTNESS_2*Vector::One).v);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::One.v);
        glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::One.v);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHTING);

        glClear(GL_DEPTH_BUFFER_BIT);
        m_camera->render();
        m_skybox->render(); // IMPORTANT: must render after camera
        renderScene();

        m_ball->renderShadow(m_lightPosition);
        for each_const(vector<Player*>, m_localPlayers, iter)
        {
            (*iter)->renderColor();
        }
        
        glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::Zero.v);
        glLightfv(GL_LIGHT1, GL_AMBIENT, (GRASS_BRIGHTNESS_2*Vector::One).v);

        m_grass->render();
    }
    else if (shadow_type == 1)
    {
        // shadow mapping

        shadowMapPass1();
        shadowMapPass2();
        shadowMapPass3();
    }

    // text messages are last
    if (m_freeze)
    {
        const Font* font = m_messages->m_fonts.find(escMessage->getFontSize())->second;
        font->begin();
        const Vector& pos = escMessage->getPosition();
        int w = font->getWidth(escMessage->getText());
        int h = font->getHeight();
        
        Vector lower = pos;
        lower.x -= w/2;
        lower.y -= 2*h;

        Vector upper = pos;
        upper.x += w/2;
        upper.y += h;

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        Video::instance->renderRoundRect(lower, upper, static_cast<float>(h/2));

        font->end();
    }
    m_messages->render();
}

void World::renderScene() const
{
    //Video::instance->renderAxes();   
    m_level->render();
}

void World::setLight(const Vector& position)
{
    m_lightPosition = position;

    glPushMatrix();
    
    glLoadIdentity();
    //gluPerspective(45.0f, 1.0f, 10.0f, 70.0f);
    glOrtho(-40.0f, 28.0f, -15.0f, 26.0f, 2.0f, 70.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, m_lightProjection.m);

    glLoadIdentity();
    gluLookAt(  m_lightPosition.x, m_lightPosition.y, m_lightPosition.z,
                4.0f, 0.0f, 5.0f,
                0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, m_lightView.m);

    glPopMatrix();

    //Calculate texture matrix for projection
    //This matrix takes us from eye space to the light's clip space
    //It is postmultiplied by the inverse of the current view matrix when specifying texgen
    static const Matrix biasMatrix( 0.5f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.5f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.5f, 0.0f,
                                    0.5f, 0.5f, 0.5f, 1.0f);    //bias from [-1, 1] to [0, 1]
    m_textureMatrix = biasMatrix * m_lightProjection * m_lightView;
}

void World::setupShadowStuff()
{
    if (Config::instance->m_video.shadow_type == 0)
    {
        return;
    }

    m_shadowSize = (1 << Config::instance->m_video.shadowmap_size) * 512; // 512, 1024, 2048

    bool valid = false;
    if (Video::instance->m_haveShadowsFB)
    {
        while (!valid)
        {
            m_framebuffer->create(m_shadowSize);
            m_framebuffer->attachColorTex();
            m_shadowTex = m_framebuffer->attachDepthTex();
            valid = m_framebuffer->isValid();
            if (!valid && m_shadowSize > 512)
            {
                m_shadowSize <<= 1;
            }
            else
            {
                break;
            }
        }
    }
    
    if (valid)
    {
        m_withFBO = true;
        m_framebuffer->unbind();
        Config::instance->m_video.shadowmap_size = (m_shadowSize / 512) >> 1;
    }
    else
    {
        m_withFBO = false;
        Config::instance->m_video.shadowmap_size = 0;
        m_shadowSize = 512;
        if (Video::instance->getResolution().second < 512)
        {
            m_shadowSize = 256;
        }

        glGenTextures(1, &m_shadowTex);
        glBindTexture(GL_TEXTURE_2D, m_shadowTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowSize, m_shadowSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        if (Video::instance->m_haveShadowsFB)
        {
            m_framebuffer->destroy();
        }
    }

    // assume depth texture is currently bound
    
    //Enable shadow comparison
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    //Shadow comparison should be true (ie not in shadow) if r<=texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    //Shadow comparison should generate an INTENSITY result
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
}

void World::killShadowStuff()
{
    if (!m_withFBO)
    {
        glDeleteTextures(1, &m_shadowTex);
    }

    m_framebuffer->destroy();
}

void World::shadowMapPass1() const
{
    //First pass - from light's point of view
    if (m_withFBO)
    {
        m_framebuffer->bind();
    }
    glPushAttrib(GL_VIEWPORT_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    //Use viewport the same size as the shadow map
    glViewport(0, 0, m_shadowSize, m_shadowSize);

    //Setup projection and view matrices from light position
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(m_lightProjection.m);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_lightView.m);

    //Disable color writes, and use flat shading for speed
    glShadeModel(GL_FLAT);
    glColorMask(0, 0, 0, 0);

    //tommeer lieli HAKi. TODO.
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(-0.1f, -0.05f);
    //glPolygonOffset(2.0f, 4.0f);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);

    //Draw the scene
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    renderScene();
    glEnable(GL_TEXTURE_2D);

    glFrontFace(GL_CW);
   
    if (!m_withFBO)
    {
        //Read the depth buffer into the shadow map texture
        glBindTexture(GL_TEXTURE_2D, m_shadowTex);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_shadowSize, m_shadowSize);
    }
  
    //restore states
    glPopAttrib();
    
    if (m_withFBO)
    {
        m_framebuffer->unbind();
    }
}

void World::shadowMapPass2() const
{
    //2nd pass - Draw from camera's point of view
    glClear(GL_DEPTH_BUFFER_BIT);

    const IntPair res = Video::instance->getResolution();

    //Setup projection and view matrices from camera position
    glViewport(0, 0, res.first, res.second);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    m_camera->render();
    m_skybox->render(); // IMPORTANT: must render after camera

    //Use dim light to represent shadowed areas
    glLightfv(GL_LIGHT1, GL_POSITION, m_lightPosition.v);
    glLightfv(GL_LIGHT1, GL_AMBIENT, (OBJECT_BRIGHTNESS_1*Vector::One).v);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, (OBJECT_BRIGHTNESS_1*Vector::One).v);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::Zero.v);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    //Draw the scene
    renderScene();

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        (*iter)->renderColor();
    }

    glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::Zero.v);
    glLightfv(GL_LIGHT1, GL_AMBIENT, (GRASS_BRIGHTNESS_1*Vector::One).v);
    m_grass->render();
}

void World::shadowMapPass3() const
{
    //3rd pass
    Video::glActiveTextureARB(GL_TEXTURE1_ARB);

    //Remember state
    glPushAttrib(GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);

    //Draw with bright light
    glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::One.v);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::One.v);

    //Set alpha test to discard false comparisons
    glAlphaFunc(GL_GEQUAL, 0.99f);
    glEnable(GL_ALPHA_TEST);

    //Bind & enable shadow map texture
    glBindTexture(GL_TEXTURE_2D, m_shadowTex);
    glEnable(GL_TEXTURE_2D);

    //Set up texture coordinate generation.
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_S, GL_EYE_PLANE, m_textureMatrix.column(0).v);
    glTexGenfv(GL_T, GL_EYE_PLANE, m_textureMatrix.column(1).v);
    glTexGenfv(GL_R, GL_EYE_PLANE, m_textureMatrix.column(2).v);
    glTexGenfv(GL_Q, GL_EYE_PLANE, m_textureMatrix.column(3).v);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);

    Video::instance->m_shadowMap3ndPass = true;

    glLightfv(GL_LIGHT1, GL_AMBIENT, (OBJECT_BRIGHTNESS_1*Vector::One).v);
    renderScene();
 
    glDisable(GL_ALPHA_TEST);

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        (*iter)->renderColor();
    }

    glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::Zero.v);
    glLightfv(GL_LIGHT1, GL_AMBIENT, (GRASS_BRIGHTNESS_2*Vector::One).v);
    m_grass->render();

    Video::instance->m_shadowMap3ndPass = false;

    Video::glActiveTextureARB(GL_TEXTURE1_ARB);

    //Restore states
    glPopAttrib();

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
}

void World::addShuffledCpuProfiles(const ProfilesVector* profiles, size_t count)
{
    vector<Profile*> temp;
    temp.assign(profiles->begin(), profiles->end());

    std::random_shuffle(temp.begin(), temp.end());
    for (size_t i = 0; i < count; i++)
    {
        m_localPlayers.push_back(new AiPlayer(temp[i], m_level));
    }
}
