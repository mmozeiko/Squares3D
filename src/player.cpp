#include "player.h"
#include "video.h"
#include "game.h"

#include "body.h" // TODO: remove

#include <cmath>

Player::Player(Game* game, int material, const Vector& pos, const Vector& size) :
    Body(game, PlayerBody),
    m_radius(size * 0.5f), m_isOnGround(true)
{
    Matrix location = Matrix::translate(pos);

    NewtonCollision* collision = NewtonCreateSphere(m_world, m_radius.x, m_radius.y, m_radius.z, NULL); 
    Body::create(collision, location);

	// set the viscous damping the the minimum
    const float damp[] = { 0.0f, 0.0f, 0.0f };
	NewtonBodySetLinearDamping(m_body, 0.0f);
	NewtonBodySetAngularDamping(m_body, damp);

	// Set Material Id for this object
	NewtonBodySetMaterialGroupID(m_body, material);

	// set the mas
    Vector intertia, origin;
    NewtonConvexCollisionCalculateInertialMatrix(collision, intertia.v, origin.v);
    NewtonBodySetMassMatrix(m_body, 10.0f, intertia.x, intertia.y, intertia.z);

  	// add and up vector constraint to help in keeping the body upright
	const Vector upDirection (0.0f, 1.0f, 0.0f);

    m_upVector = NewtonConstraintCreateUpVector(m_world, upDirection.v, m_body); 

    m_texture = m_game->m_video->loadTexture("player.tga");
}

Player::~Player()
{
    glDeleteTextures(1, &m_texture);
    NewtonDestroyJoint(m_world, m_upVector);
}

void Player::setDirection(const Vector& direction)
{
    m_direction = direction;
}

void Player::setRotation(const Vector& rotation)
{
    m_rotation = rotation;
}

void Player::render(const Video* video) const
{
    video->beginObject(m_matrix);

    glColor3f(0.5f, 0.5f, 0.5f);
    glScalef(m_radius.x, m_radius.y, m_radius.z);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glPushMatrix();
    glMatrixMode(GL_TEXTURE_MATRIX);
    glRotatef(-90.0, 1, 0, 0);
    glRotatef(90.0, 0, 0, 1);

    video->renderSphere(1.0f);

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    video->endObject();
}

void Player::onSetForceAndTorque()
{
	float timestepInv = 1.0f / 0.01f; // 0.01f == DT

    float mass;
    float Ixx, Iyy, Izz;

    // Get the mass of the object
    NewtonBodyGetMassMatrix(m_body, &mass, &Ixx, &Iyy, &Izz );

    Vector force = gravityVec * mass;

    Vector currentVel;
    NewtonBodyGetVelocity(m_body, currentVel.v);
      
    Vector targetVel = m_direction;
    targetVel.norm();
    if (!m_isOnGround)
    {
       NewtonBodyAddForce(m_body, force.v);
    }

    Matrix m;
    
    //m = m_matrix;
    //m.m30 = m.m31 = m.m32 = 0.0f;

    force = (targetVel * 5.0f - currentVel ) * timestepInv * mass;
    if (!m_isOnGround) force.y = 0.0f;

    NewtonBodyAddForce(m_body, force.v);

    m_isOnGround = false;
  
    Vector omega;
	NewtonBodyGetOmega(m_body, omega.v);

	Vector torque = m_rotation;
    torque.norm();
    torque = (10.0f * torque - omega) * timestepInv * Iyy;
	NewtonBodySetTorque (m_body, torque.v);
}

void Player::onCollision(const NewtonMaterial* material, const NewtonContact* contact)
{
   Vector pos, nor;

   NewtonMaterialGetContactPositionAndNormal(material, pos.v, nor.v);

   // Determine if this contact is on the ground
   Vector dir(0.0f, 1.0f, 0.0f);
   float angle = dir % nor;
   m_isOnGround = (angle > 0.0f);


   NewtonMaterialSetContactElasticity( material, 0.0f );

   NewtonMaterialSetContactFrictionState( material, 0, 0 );
   NewtonMaterialSetContactFrictionState( material, 0, 1 );

   NewtonMaterialSetContactSoftness( material, 0 );

}
