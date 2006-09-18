#include "intro.h"
#include "input.h"
#include "texture.h"
#include "font.h"

static const float FADE_IN_SECS = 2.0f;
static const float BALL_KICK_SECS = 5.0f;
static const float FADE_OUT_SECS = 16.0f;
static const float MENU_SHOW_SECS = 18.0f;

static const float PIECE_MASS = 1.0f;
static const float PIECE_SIZEX = 0.3f;
static const float PIECE_SIZEY = 0.3f;
static const float PIECE_SIZEZ = 0.03f;

static const int PIECE_XCOUNT = 10;
static const int PIECE_YCOUNT = 6;

static const float BALL_MASS = 5000.0f;
static const float BALL_R = 0.1f;

static void OnForce(const NewtonBody* body)
{
    Vector gravity( 0.0f, -9.81f, 0.0f );
    float m, Ix, Iy, Iz;
    NewtonBodyGetMassMatrix(body, &m, &Ix, &Iy, &Iz);
    gravity.y *= m;
    NewtonBodyAddForce(body, gravity.v);
}

Intro::Intro() : m_timePassed(0), m_nextState(false), m_ballKicked(false), m_font(Font::get("Arial_32pt_bold"))
{
    m_newtonWorld = NewtonCreate(NULL, NULL);
    NewtonSetSolverModel(m_newtonWorld, 10);
    NewtonSetFrictionModel(m_newtonWorld, 1);

    vector<UV> piece_uv;

    NewtonCollision* collision = NewtonCreateBox(m_newtonWorld, PIECE_SIZEX, PIECE_SIZEY, PIECE_SIZEZ, NULL);
    for (int y = 0; y < PIECE_YCOUNT; y++)
    {
        float inertia[3];
        float origin[3];
        for (int x = 0; x < PIECE_XCOUNT; x++)
        {
            NewtonBody* body = NewtonCreateBody(m_newtonWorld, collision);
            Matrix matrix = Matrix::translate(
                Vector(-PIECE_XCOUNT*PIECE_SIZEX/2+PIECE_SIZEX/2 + x*PIECE_SIZEX + (y%2 ? PIECE_SIZEX/2.0f : 0), 
                       PIECE_SIZEY/2.0f + y * PIECE_SIZEY,
                       0.0f)
            );

            NewtonBodySetMatrix(body, matrix.m);

            NewtonConvexCollisionCalculateInertialMatrix(collision, inertia, origin);
            inertia[0] *= PIECE_MASS;
            inertia[1] *= PIECE_MASS;
            inertia[2] *= PIECE_MASS;
            NewtonBodySetMassMatrix(body, PIECE_MASS, inertia[0], inertia[1], inertia[2]);
            NewtonBodySetCentreOfMass(body, origin);
            NewtonBodySetForceAndTorqueCallback(body, &OnForce);

            NewtonBodySetAutoFreeze(body, 1);
            NewtonWorldFreezeBody(m_newtonWorld, body);

            m_piece_body.push_back(body);
            m_piece_matrix.push_back(matrix);

            static const float maxv = 768.0f/1024.0f; // depends on indago logo texture structure

            static const float uv[][2] = {
                { 0.0f, 1.0f },
                { 1.0f, 1.0f },
                { 1.0f, 0.0f },
                { 0.0f, 0.0f },
            };

            for (int k=0; k<4; k++)
            {
                float tmpX = static_cast<float>(x);
                if ((y&1)==1)
                {
                    tmpX += 0.5f;
                }
                float u = 1.0f-(tmpX+uv[k][0])/PIECE_XCOUNT;
                float v = (y+uv[k][1])*maxv/PIECE_YCOUNT;
                piece_uv.push_back(UV(u, v));
            }
        }
    }
    NewtonReleaseCollision(m_newtonWorld, collision);

    // ball
    collision = NewtonCreateSphere(m_newtonWorld, BALL_R, BALL_R, BALL_R, NULL);
    m_ball_body = NewtonCreateBody(m_newtonWorld, collision);

    Matrix matrix = Matrix::translate(Vector(0.0f, BALL_R, 2.0f));
    NewtonBodySetMatrix(m_ball_body, matrix.m);

    float inertia[3];
    float origin[3];
    NewtonConvexCollisionCalculateInertialMatrix(collision, inertia, origin);
    inertia[0] *= BALL_MASS;
    inertia[1] *= BALL_MASS;
    inertia[2] *= BALL_MASS;
    NewtonBodySetMassMatrix(m_ball_body, BALL_MASS, inertia[0], inertia[1], inertia[2]);
    NewtonBodySetCentreOfMass(m_ball_body, origin);
    NewtonBodySetForceAndTorqueCallback(m_ball_body, &OnForce);

    NewtonBodySetAutoFreeze(m_ball_body, 1);
    NewtonReleaseCollision(m_newtonWorld, collision);

    // floor
    NewtonCollision* floor = NewtonCreateBox(m_newtonWorld, 20.0f, 1.0f, 20.0f, NULL);
    NewtonBody* floorBody = NewtonCreateBody(m_newtonWorld, floor);
    NewtonReleaseCollision(m_newtonWorld, floor);
    matrix = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
    NewtonBodySetMatrix(floorBody, matrix.m);

    m_ballTex = Video::instance->loadTexture("ball");
    m_logoTex = Video::instance->loadTexture("indago_logo");
    m_bigLogoTex = Video::instance->loadTexture("logo_big");

    m_cubeListBase = glGenLists(PIECE_XCOUNT * PIECE_YCOUNT);
    // -0.5 .. 0.5
    static const float vertices[][3] = {
        /* 0 */ { -0.5f, -0.5f, -0.5f },
        /* 1 */ {  0.5f, -0.5f, -0.5f },
        /* 2 */ {  0.5f, -0.5f,  0.5f },
        /* 3 */ { -0.5f, -0.5f,  0.5f },

        /* 4 */ { -0.5f, 0.5f, -0.5f },
        /* 5 */ {  0.5f, 0.5f, -0.5f },
        /* 6 */ {  0.5f, 0.5f,  0.5f },
        /* 7 */ { -0.5f, 0.5f,  0.5f },
    };

    static const int faces[][4] = {
        { 0, 1, 2, 3 }, // bottom
        { 4, 7, 6, 5 }, // up
        { 4, 5, 1, 0 }, // front
        { 6, 7, 3, 2 }, // back
        { 7, 4, 0, 3 }, // left
        { 5, 6, 2, 1 }, // right
    };
    
    static const float normals[][3] = {
        {  0.0f, -1.0f,  0.0f }, // bottom
        {  0.0f,  1.0f,  0.0f }, // up
        {  0.0f,  0.0f, -1.0f }, // front
        {  0.0f,  0.0f,  1.0f }, // back
        { -1.0f,  0.0f,  0.0f }, // left
        {  1.0f,  0.0f,  0.0f }, // right
    };

    for (int c = 0; c < PIECE_XCOUNT * PIECE_YCOUNT; c++)
    {
        glNewList(m_cubeListBase + c, GL_COMPILE);
        glBegin(GL_QUADS);
        for (int i = 0; i < 6; i++)
        {
            glNormal3fv(normals[i]);
            for (int k = 0; k < 4; k++)
            {
                glTexCoord2fv(piece_uv[4*c+k].uv);
                glVertex3fv(vertices[faces[i][k]]);
            }
        }
        glEnd();
        glEndList();
    }

    Input::instance->startKeyBuffer();
    Input::instance->startButtonBuffer();
}

Intro::~Intro()
{
    Input::instance->endKeyBuffer();
    Input::instance->endButtonBuffer();

    glDeleteLists(m_cubeListBase, PIECE_XCOUNT * PIECE_YCOUNT);

    Video::instance->unloadTextures();
    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);
}

void Intro::control()
{
    m_nextState = Input::instance->popButton() != -1;

    if (Input::instance->popKey() != -1 && m_timePassed < FADE_OUT_SECS)
    {
        if (m_timePassed > FADE_IN_SECS)
        {
            m_timePassed = FADE_OUT_SECS;
        }
        else
        {
            m_timePassed = FADE_OUT_SECS + (FADE_IN_SECS - m_timePassed);
        }
        m_ballKicked = true;
    }
    if (m_timePassed > BALL_KICK_SECS && !m_ballKicked)
    {
        Matrix matrix = Matrix::translate(Vector(0.0f, BALL_R, 2.0f));
        NewtonBodySetMatrix(m_ball_body, matrix.m);
        NewtonBodySetVelocity(m_ball_body, Vector(0.0f, 5.5f, -3.5f).v);
        NewtonWorldUnfreezeBody(m_newtonWorld, m_ball_body);
        
        m_ballKicked = true;
    }
}

void Intro::update(float delta)
{
    m_timePassed += delta;

    if (m_timePassed > MENU_SHOW_SECS)
    {
        m_nextState = true;
    }
}

void Intro::updateStep(float delta)
{
    NewtonUpdate(m_newtonWorld, delta);
}

void Intro::prepare()
{
    for (size_t i=0; i<m_piece_body.size(); i++)
    {
        NewtonBodyGetMatrix(m_piece_body[i], m_piece_matrix[i].m);
    }
    NewtonBodyGetMatrix(m_ball_body, m_ball_matrix.m);
}

void Intro::render() const
{
    glClear(GL_DEPTH_BUFFER_BIT);

    // camera

    glLoadIdentity();
    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, -0.85f, 2.0f);

    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
        
    glLightfv(GL_LIGHT1, GL_POSITION, Vector(-15.0f, 3.0f, -13.0f).v);
    glLightfv(GL_LIGHT1, GL_AMBIENT, (0.25f*Vector::One).v);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, Vector::One.v);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Vector::One.v);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    // cubes

    glDisable(GL_CULL_FACE); // TODO: hack, remove it
    m_logoTex->begin();
    int i = 0;
    for each_const(vector<Matrix>, m_piece_matrix, iter)
    {
        glPushMatrix();
        glMultMatrixf(iter->m);
        glScalef(PIECE_SIZEX, PIECE_SIZEY, PIECE_SIZEZ);
        glCallList(m_cubeListBase + i++);
        glPopMatrix();
    }
    m_logoTex->end();

    // floor

/*    static const float gray[] = { 0.2f, 0.2f, 0.2f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gray);

    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -0.01f, -10.0f);
    glVertex3f(-10.0f, -0.01f,  10.0f);
    glVertex3f( 10.0f, -0.01f,  10.0f);
    glVertex3f( 10.0f, -0.01f, -10.0f);
    glEnd();
*/
    
    // ball

    glPushMatrix();
    glMultMatrixf(m_ball_matrix.m);
    m_ballTex->begin();
    Video::instance->renderSphere(BALL_R);
    m_ballTex->end();
    glPopMatrix();

    // fade in
    if (m_timePassed < FADE_IN_SECS)
    {
        float alpha = m_timePassed / FADE_IN_SECS;
        alpha = alpha*alpha;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, alpha);

        glBegin(GL_QUADS);
        glVertex3f(5.0f, -1.0f, -1.8f);
        glVertex3f(5.0f, 5.0f, -1.8f);
        glVertex3f(-5.0f, 5.0f, -1.8f);
        glVertex3f(-5.0f, -1.0f, -1.8f);
        glEnd();

        glPopAttrib();
    }

    if (m_timePassed > BALL_KICK_SECS)
    {
        m_bigLogoTex->begin();

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(5.0f, -1.0f, 7.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.0f, -1.0f, 7.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-5.0f, 5.0f, 7.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(5.0f, 5.0f, 7.0f);
        glEnd();

        m_bigLogoTex->end();
    }

    if (m_timePassed > FADE_OUT_SECS)
    {
        float alpha = (MENU_SHOW_SECS-m_timePassed) / (MENU_SHOW_SECS - FADE_OUT_SECS);
        alpha = alpha*alpha;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, alpha);

        glBegin(GL_QUADS);
        glVertex3f(5.0f, -1.0f, -1.8f);
        glVertex3f(5.0f, 5.0f, -1.8f);
        glVertex3f(-5.0f, 5.0f, -1.8f);
        glVertex3f(-5.0f, -1.0f, -1.8f);
        glEnd();

        glPopAttrib();
    }

    glEnable(GL_CULL_FACE); // TODO: hack, remove it

    glPopAttrib();
}

State::Type Intro::progress() const
{
    if (m_nextState)
    {
        return State::Menu;
    }
    return State::Current;
}
