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

World* System<World>::instance = NULL;

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
                                Vector(0.0f, 0.0f, 1.0f),
                                Font::Align_Center);
                m_messages->add2D(escMessage);
            }
        }

        if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) && m_freeze) 
        {
            Input::instance->endKeyBuffer();
            return State::Menu;
        }
    }
    while (key != -1);

    return State::Current;
}

World::World() : 
    m_freeze(false),
    escMessage(NULL),
    m_music(NULL),
    m_camera(NULL),
    m_skybox(NULL),
    m_newtonWorld(NULL),
    m_level(NULL),
    m_ball(NULL),
    m_referee(NULL),
    m_messages(NULL),
    m_scoreBoard(NULL),
    m_framebuffer(NULL)
{
    setInstance(this); // MUST go first

    m_framebuffer = new FrameBuffer();
    setupShadowStuff();
    setLight(Vector(-15.0f, 30.0f, 13.0f));

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
    m_level->load("level.xml");

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_referee = new Referee(m_messages, m_scoreBoard);
    m_referee->m_ground = m_level->getBody("level");

    m_ball = new Ball(m_level->getBody("football"));
    m_referee->registerBall(m_ball);

    Player* human = new LocalPlayer("penguin3", Vector(-1.5f, 1.0f, -1.5f), Vector::Zero);
    m_localPlayers.push_back(human);

    m_referee->registerPlayer("player1", human);
    m_ball->addBodyToFilter(human->m_body);

    int i = 0;
    //todo: position ai players without such hacks
    for (float x = -1.5f; x <= 1.5f; x += 3.0f)
    { 
        for (float z = 1.5f; z >= -1.5f; z -= 3.0f)
        { 
            if ((x != -1.5f) || (z != -1.5f))
            {
                Vector pos(x, 1.0f, z);
                Player* ai = new AiPlayer("penguin" + cast<string>(i), pos, Vector::Zero);
                m_localPlayers.push_back(ai);

                m_referee->registerPlayer("ai_player" + cast<string>(i), m_localPlayers.back());
                m_ball->addBodyToFilter(ai->m_body);
                i++;
            }
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

    m_music->stop();
    Audio::instance->unloadMusic(m_music);

    for each_const(vector<Player*>, m_localPlayers, player)
    {
        delete *player;
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
    }
}

void World::update(float delta)
{
    // update is called one time in frame

    m_camera->update(delta);
    m_scoreBoard->update();
    m_messages->update(delta);
    m_referee->update();
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
        glLightfv(GL_LIGHT1, GL_AMBIENT, (0.25f*Vector::One).v);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::One.v);
        glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::One.v);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHTING);

        glClear(GL_DEPTH_BUFFER_BIT);
        m_camera->render();
        m_skybox->render(); // IMPORTANT: must render after camera
        renderScene();

        m_ball->renderShadow(m_lightPosition);
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
        m_messages->m_font->begin();
        const Vector& pos = escMessage->getPosition();
        int w = m_messages->m_font->getWidth(escMessage->getText());
        int h = m_messages->m_font->getHeight();
        
        Vector lower = pos;
        lower.x -= w/2;
        lower.y -= 2*h;

        Vector upper = pos;
        upper.x += w/2;
        upper.y += h;

        glDisable(GL_TEXTURE_2D);
        Video::instance->renderRoundRect(lower, upper, static_cast<float>(h/2));
        glEnable(GL_TEXTURE_2D);
        m_messages->m_font->end();
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
    gluPerspective(50.0f, 1.0f, 1.0f, 52.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, m_lightProjection.m);

    glLoadIdentity();
    gluLookAt(  m_lightPosition.x, m_lightPosition.y, m_lightPosition.z,
                0.0f, 0.0f, 0.0f,
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

    m_shadowSize = Config::instance->m_video.shadowmap_size;

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
        Config::instance->m_video.shadowmap_size = m_shadowSize;
    }
    else
    {
        m_withFBO = false;
        m_shadowSize = Config::instance->m_video.shadowmap_size = 512;
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
    //glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-0.25f, -0.01f); 
    //glPolygonOffset(1.0f, 2.0f);

    //Draw the scene
    glDisable(GL_TEXTURE_2D);
    renderScene();
    glEnable(GL_TEXTURE_2D);
   
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
    glLightfv(GL_LIGHT1, GL_AMBIENT, (0.25f*Vector::One).v);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, (0.25f*Vector::One).v);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::Zero.v);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    //Draw the scene
    renderScene();
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
    renderScene();
    Video::glActiveTextureARB(GL_TEXTURE1_ARB);

    //Restore states
    glPopAttrib();

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
}
