#include "openal_includes.h"

#include "world.h"
#include "player.h"
#include "camera.h"
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
#include "referee_local.h"
#include "referee_base.h"
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
#include "game.h"
#include "fence.h"
#include "random.h"
#include "hdr.h"
#include "chat.h"
#include "shader.h"

static const float OBJECT_BRIGHTNESS_1 = 0.5f; // shadowed
static const float OBJECT_BRIGHTNESS_2 = 0.6f; // lit
static const float GRASS_BRIGHTNESS_1 = 0.5f; // shadowed
static const float GRASS_BRIGHTNESS_2 = 0.5f; // lit

static const Vector playerPositions[4] = {Vector(- FIELD_LENGTH / 2, 1.5f, - FIELD_LENGTH / 2),
                                          Vector(- FIELD_LENGTH / 2, 1.5f,   FIELD_LENGTH / 2),
                                          Vector(  FIELD_LENGTH / 2, 1.5f,   FIELD_LENGTH / 2),
                                          Vector(  FIELD_LENGTH / 2, 1.5f, - FIELD_LENGTH / 2)
                                          };

template <class World> World* System<World>::instance = NULL;

State::Type World::progress()
{
    // TODO: move key reading to update
    int key;
    do
    {
        key = Input::instance->popKey();
        
        if (Network::instance->m_isSingle == false)
        {
            if (m_chat->updateKey(key))
            {
                continue;
            }
        }

        if (key == GLFW_KEY_ESC)
        {
            if (!Network::instance->m_isSingle && (!Network::instance->m_needToBeginGame || Network::instance->m_needToQuitGame))
            {
                if (m_networkPaused)
                {
                    m_messages->remove(m_escMessage);
                    m_escMessage = NULL;
                    m_networkPaused = false;
                }
                else
                {
                    m_networkPaused = true;
                    float resY = static_cast<float>(Video::instance->getResolution().second);
                    m_escMessage = new Message(
                                    Language::instance->get(TEXT_NETWORK_ESC_MESSAGE), 
                                    Vector(static_cast<float>(Video::instance->getResolution().first) / 2, 
                                           resY / 2 + resY / 3, 
                                           0.0f), 
                                    White,
                                    Font::Align_Center);
                    m_messages->add2D(m_escMessage);
                }
            }

            else // local game
            {
                if (m_freeze)
                {
                    m_freeze = false;
                    m_messages->remove(m_escMessage);
                    m_escMessage = NULL;
                    return State::Current;
                }
                else
                {
                    m_freeze = true;
                    float resY = static_cast<float>(Video::instance->getResolution().second);
                    m_escMessage = new Message(
                                    Language::instance->get(TEXT_ESC_MESSAGE), 
                                    Vector(static_cast<float>(Video::instance->getResolution().first) / 2, 
                                           resY / 2 + resY / 3, 
                                           0.0f), 
                                    White,
                                    Font::Align_Center);
                    m_messages->add2D(m_escMessage);
                }
            }
        }

        if (Network::instance->m_isSingle)
        {
            if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)) 
            {
                State::Type returnState = State::Current;
                if (m_referee->m_gameOver)
                {
                    if (m_referee->getLoserName() != m_userProfile->m_name) 
                    {
                        if ((m_current == m_unlockable) && (m_unlockable < 3))
                        {
                            m_unlockable++;
                        }
                        returnState = State::Menu;
                    }
                    else
                    {
                        //here we don`t change the unlockable
                        if (m_freeze)
                        {
                            //user wants to leave to menu and doesn`t want to retry - return State::MenuEasy
                            returnState = State::Menu;
                        }
                        else
                        {
                            //if none from above - we return State::m_current to retry (reload the same level)
                            init();
                        }
                    }
                }
                else
                {
                    if (m_freeze)
                    {
                        //user just wants to leave to menu in the middle of game - return State::MenuEasy
                        //it DOESN`T change the unlockable level
                        returnState = State::Menu;
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
        else // networked
        {
            if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)) 
            {
                if (m_networkPaused)
                {
                    m_messages->remove(m_escMessage);
                    m_escMessage = NULL;
                    return State::Menu;
                }
            }
        }
    }
    while (key != -1);

    if (!Network::instance->m_isSingle)
    {
        if (Network::instance->m_needToBeginGame)
        {
            m_freeze = false;
            m_messages->remove(m_waitMessage);
            m_waitMessage = NULL;
            
            Network::instance->m_needToBeginGame = false;
        }
        if (Network::instance->m_disconnected)
        {
            return State::Menu;
        }
    }

    return State::Current;
}

World::World(Profile* userProfile, int& unlockable, int current) :
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
    m_escMessage(NULL),
    m_framebuffer(NULL),
    m_unlockable(unlockable),
    m_current(current),
    m_waitMessage(NULL),
    m_hdr(NULL),
    m_chat(NULL),
    m_networkPaused(false),
    m_shadowShader(NULL)
{
    setInstance(this); // MUST go first

    m_framebuffer = new FrameBuffer();

    m_chat = new Chat(m_userProfile->m_name, m_userProfile->m_color);
    m_hdr = new HDR();
    m_hdr->init();

    setupShadowStuff();
    setLight(Vector(-15.0f, 35.0f, 38.0f, 0.0f));

    Input::instance->startCharBuffer();
    Input::instance->startKeyBuffer();
}

void World::init()
{
    if (m_newtonWorld != NULL)
    {
        delete m_skybox;

        delete m_messages;
        delete m_scoreBoard;

        for each_const(vector<Player*>, m_localPlayers, iter)
        {
            delete *iter;
        }
        m_localPlayers.clear();
       
        delete m_ball;
        delete m_referee;
        delete m_grass;
        delete m_level;
        delete m_camera;

        NewtonDestroyAllBodies(m_newtonWorld);
        NewtonDestroy(m_newtonWorld);

        m_skybox = NULL;

        m_messages = NULL;
        m_scoreBoard = NULL;

        m_ball = NULL;
        m_referee = NULL;
        m_level = NULL;
        m_newtonWorld = NULL;
        m_camera = NULL;
    }

    m_messages = new Messages();
    m_scoreBoard = new ScoreBoard(m_messages);

    m_newtonWorld = NewtonCreate(NULL, NULL);

    // enable some Newton optimization
    NewtonSetSolverModel(m_newtonWorld, 1);
    
    m_level = new Level();

    StringSet tmp;
    if (Network::instance->m_isSingle)
    {
        m_level->load( ( m_current < 3 ? "world.xml" : "extra.xml" ), tmp);
    }
    else
    {
        m_level->load( Network::instance->getLevel(), tmp);
    }
    m_grass = new Grass(m_level);
    
    if (m_level->m_fences.empty() == false)
    {
        makeFence(m_level, m_newtonWorld);
    }

    m_skybox = new SkyBox(m_level->m_skyboxName);
    m_hdr->updateFromLevel(m_level->m_hdr_eps, m_level->m_hdr_exp, m_level->m_hdr_mul);

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    const vector<Player*>& players = Network::instance->createPlayers(m_level);

    m_localPlayers = players;

    m_ball = new Ball(m_level->getBody("football"), m_level->m_collisions["level"]);

    if (Network::instance->m_isSingle || Network::instance->m_isServer)
    {
        m_referee = new RefereeLocal(m_messages, m_scoreBoard);
        m_referee->m_field = m_level->getBody("field"); //referee now can recognize game field
        m_referee->m_ground = m_level->getBody("level");; //referee now can recognize ground outside
        
        //this is for correct registering when waiting for ball bounce in referee
        //it is handled specifically in Ball OnCollide
        m_ball->addBodyToFilter(m_referee->m_field);
        m_ball->addBodyToFilter(m_referee->m_ground);

        m_referee->registerBall(m_ball);

        m_referee->m_humanPlayer = players[Network::instance->getLocalIdx()];
    }
    else
    {
        //client referee
        m_referee = new RefereeBase(m_messages, m_scoreBoard);
    }

    m_referee->registerPlayers(m_localPlayers);

    for (size_t i = 0; i < m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
        m_ball->addBodyToFilter(m_localPlayers[i]->m_body);
    }

    if (!Network::instance->m_isSingle)
    {
        Network* net = Network::instance;
        net->setChat(m_chat);

        net->setReferee(m_referee);

        net->add(m_ball->m_body);

        for each_const(vector<Player*>, players, iter)
        {
            net->add((*iter)->m_body);
        }

        net->add(m_level->getBody("field"));
        net->add(m_level->getBody("level"));

        NewtonBodySetMassMatrix(m_level->getBody("seat")->m_newtonBody, 0, 0, 0, 0);
        NewtonBodySetMassMatrix(m_level->getBody("cucumberFan1")->m_newtonBody, 0, 0, 0, 0);
        NewtonBodySetMassMatrix(m_level->getBody("cucumberFan2")->m_newtonBody, 0, 0, 0, 0);
        NewtonBodySetMassMatrix(m_level->getBody("cucumberFan3")->m_newtonBody, 0, 0, 0, 0);
    }

    m_scoreBoard->reset();

    if (!m_level->m_music.empty())
    {
        m_level->m_music[Randoms::getIntN(static_cast<unsigned int>(m_level->m_music.size()))]->play();
    }

    Network::instance->iAmReady();

    if (!Network::instance->m_isSingle)
    {
        m_freeze = true;
    
        float resY = static_cast<float>(Video::instance->getResolution().second);
        m_waitMessage = new Message(
                        Language::instance->get(TEXT_WAIT_PLAYERS),
                        Vector(static_cast<float>(Video::instance->getResolution().first / 2), 
                               resY / 2 + resY / 4, 
                               0.0f), 
                        Blue,
                        Font::Align_Center);
        m_messages->add2D(m_waitMessage);    
    }

    m_referee->m_sound->play(m_referee->m_soundGameStart);

    float angleAdjust = Network::instance->getLocalIdx() * 90.0f;
    m_camera = new Camera(Vector(0.0f, 1.0f, 12.0f), 20.0f, 0.0f + angleAdjust);
}

World::~World()
{  
    // NETWORK-
    
    Network::instance->close();
    
    // -NETWORK

    killShadowStuff();
    delete m_framebuffer;
    if (m_grass != NULL)
    {
        delete m_grass;
    }

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        delete *iter;
    }

    delete m_ball;
    delete m_referee;
    delete m_level;

    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

    delete m_chat;
    delete m_hdr;
    delete m_scoreBoard;
    delete m_messages;
    delete m_skybox;
    delete m_camera;
    
    Input::instance->endCharBuffer();
    Input::instance->endKeyBuffer();
}

void World::control()
{
    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE)
    {
        // only camera and local players
        m_camera->control();
        if (Network::instance->m_isSingle)
        {
            m_localPlayers[Network::instance->getLocalIdx()]->control();
        }
    }
    if (!Network::instance->m_isSingle)
    {
        m_localPlayers[Network::instance->getLocalIdx()]->control();
    }
    
    // other objects go after this

    for (size_t i=0; i<m_localPlayers.size(); i++)
    {
        if (i != Network::instance->getLocalIdx())
        {
            m_localPlayers[i]->control();
        }
    }
}

void World::updateStep(float delta)
{
    // updateStep is called more than one time in frame
    m_level->m_properties->update();
    //

    if (Network::instance->m_isSingle == false && Network::instance->m_isServer == false)
    {
        return;
    }

    if (!m_freeze)
    {
        m_ball->triggerBegin();
        NewtonUpdate(m_newtonWorld, delta);
        m_ball->triggerEnd();

        for (size_t i=0; i<m_localPlayers.size(); i++)
        {
            if (m_localPlayers[i]->getPosition().y < -5.0f)
            {
                m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
                NewtonWorldUnfreezeBody(m_newtonWorld, m_localPlayers[i]->m_body->m_newtonBody);
            }
        }

        if (m_ball->getPosition().y < -5.0f || 
            std::abs(m_ball->getPosition().x) > 80.0f ||
            std::abs(m_ball->getPosition().y) > 80.0f ||
            std::abs(m_ball->getPosition().z) > 80.0f)
        {
            if (m_referee->m_gameOver)
            {
                m_ball->setPosition0();
                NewtonWorldUnfreezeBody(m_newtonWorld, m_ball->m_body->m_newtonBody);
                NewtonBodySetVelocity(m_ball->m_body->m_newtonBody, Vector::Zero.v);
                NewtonBodySetOmega(m_ball->m_body->m_newtonBody, Vector::Zero.v);
            }
            else
            {
                m_referee->process(m_ball->m_body, m_level->getBody("level")); 
            }
        }

    }
}

void World::update(float delta)
{
    // update is called one time in frame

    alListenerfv(AL_POSITION, m_localPlayers[0]->getPosition().v);
    alListenerfv(AL_VELOCITY, m_localPlayers[0]->m_body->getVelocity().v);

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
        m_hdr->begin();
        glClear(GL_DEPTH_BUFFER_BIT);
        m_camera->render();
        m_skybox->render(); // IMPORTANT: must render after camera

        glLightfv(GL_LIGHT1, GL_POSITION, m_lightPosition.v);
        glLightfv(GL_LIGHT1, GL_AMBIENT, (OBJECT_BRIGHTNESS_1*Vector::One).v);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, (OBJECT_BRIGHTNESS_2*Vector::One).v);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHTING);

        renderScene();

        m_ball->renderShadow(m_lightPosition);
        for each_const(vector<Player*>, m_localPlayers, iter)
        {
            (*iter)->renderColor();
        }
        
        glLightfv(GL_LIGHT1, GL_DIFFUSE, (GRASS_BRIGHTNESS_1*Vector::One).v);
        glLightfv(GL_LIGHT1, GL_AMBIENT, (GRASS_BRIGHTNESS_2*Vector::One).v);

        m_grass->render();
    }
    else if (shadow_type == 1)
    {
        // shadow mapping

        shadowMapPass1();
        m_hdr->begin();
        shadowMapPass2();
    }
    m_hdr->end();
    m_hdr->render();

    // text messages are last
    if (m_freeze || m_networkPaused)
    {
        if (m_escMessage != NULL || m_waitMessage != NULL)
        {
            Message* m = m_escMessage;
            if (m == NULL)
            {
                 m = m_waitMessage;
            }
            int i = m->getFontSize();
            const Font* font = m_messages->m_fonts.find(i)->second;
            font->begin();

            float w2 = static_cast<float>(font->getWidth(m->getText())) / 2.0f;
            float h = static_cast<float>(font->getHeight(m->getText()));

            const Vector& pos = m->getPosition();

            Vector lower(pos.x - w2, pos.y - h + i, 0.0f);
            Vector upper(pos.x + w2, pos.y + i, 0.0f);

            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            Video::instance->renderRoundRect(lower, upper, i / 2.0f);

            font->end();
        }
    }

    if (Network::instance->m_isSingle == false)
    {
        m_chat->render();
    }
    if (m_referee->m_over != NULL)
    {
        Message* m = m_referee->m_over;
        const Font* font = m_messages->m_fonts.find(m->getFontSize())->second;
        font->begin();
        float w = static_cast<float>(font->getWidth(m->getText()));
        float h = static_cast<float>(font->getHeight(m->getText()));

        const Vector& pos = m->getPosition() - Vector(0.0f, h / 4, 0.0f);

        Vector lower = pos;
        lower.x -= w / 2;
        lower.y -= h / 2;

        Vector upper = pos;
        upper.x += w / 2;
        upper.y +=  h / 2;

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
        Video::instance->renderRoundRect(lower, upper, h / 4);

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

    if (Config::instance->m_video.shadow_type == 0)
    {
        return;
    }

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

    //Bind & enable shadow map texture
    glActiveTextureARB(GL_TEXTURE1_ARB);
    //Set up texture coordinate generation.
    glTexGenfv(GL_S, GL_EYE_PLANE, m_textureMatrix.column(0).v);
    glTexGenfv(GL_T, GL_EYE_PLANE, m_textureMatrix.column(1).v);
    glTexGenfv(GL_R, GL_EYE_PLANE, m_textureMatrix.column(2).v);
    glTexGenfv(GL_Q, GL_EYE_PLANE, m_textureMatrix.column(3).v);
    glActiveTextureARB(GL_TEXTURE0_ARB);

}

void World::setupShadowStuff()
{
    m_shadowShader = new Shader("shadow");
    m_shadowShader->setInt1("tex_source", 0);
    m_shadowShader->setInt1("tex_shadow", 1);
    m_shadowShader->end();
    
    if (m_shadowShader->valid() == false)
    {
        Config::instance->m_video.shadow_type = 0;
    }

    if (Config::instance->m_video.shadow_type == 0)
    {
        return;
    }

    m_shadowSize = (1 << Config::instance->m_video.shadowmap_size) * 512; // 512, 1024, 2048

    bool valid = false;

    while (!valid)
    {
        m_framebuffer->create(m_shadowSize, m_shadowSize);
        m_shadowTex = m_framebuffer->attachDepthTex();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        valid = m_framebuffer->isValid();
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        if (!valid && m_shadowSize > 512)
        {
            m_shadowSize <<= 1;
        }
        else
        {
            break;
        }
    }
    
    if (valid)
    {
        m_framebuffer->unbind();
        Config::instance->m_video.shadowmap_size = (m_shadowSize / 512) >> 1;
    }
    else
    {
        Config::instance->m_video.shadow_type = 0;
        m_framebuffer->destroy();
        return;
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
    if (Config::instance->m_video.shadow_type != 0)
    {
        glDeleteTextures(1, (GLuint*)&m_shadowTex);
    }

    m_framebuffer->destroy();
    if (m_shadowShader != NULL)
    {
        delete m_shadowShader;
    }
}

void World::shadowMapPass1() const
{
    //First pass - from light's point of view
    if (Config::instance->m_video.shadow_type != 0)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
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
   
    //restore states
    glPopAttrib();
    
    if (Config::instance->m_video.shadow_type != 0)
    {
        m_framebuffer->unbind();
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
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

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);

    static const Vector amb1 = Vector::One * 0.6f;
    static const Vector dif1 = Vector::One * 0.75f;
    static const Vector amb2 = Vector::One * 0.6f;
    static const Vector dif2 = Vector::One * 0.6f;
    //Use dim light to represent shadowed areas
    glLightfv(GL_LIGHT1, GL_POSITION, m_lightPosition.v);
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb1.v);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dif1.v);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    //Bind & enable shadow map texture
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_shadowTex);
    //Set up texture coordinate generation.
    glTexGenfv(GL_S, GL_EYE_PLANE, m_textureMatrix.column(0).v);
    glTexGenfv(GL_T, GL_EYE_PLANE, m_textureMatrix.column(1).v);
    glTexGenfv(GL_R, GL_EYE_PLANE, m_textureMatrix.column(2).v);
    glTexGenfv(GL_Q, GL_EYE_PLANE, m_textureMatrix.column(3).v);
    glActiveTextureARB(GL_TEXTURE0_ARB);

    m_shadowShader->begin();
    renderScene();

    glLightfv(GL_LIGHT1, GL_AMBIENT, amb2.v);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dif2.v);
    m_grass->render();
    m_shadowShader->end();

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        (*iter)->renderColor();
    }

    //Restore states
    glPopAttrib();
}
