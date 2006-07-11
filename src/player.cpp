#include "player.h"
#include "video.h"

#include "body.h" // TODO: remove

#include <cmath>

Player::Player(const NewtonWorld* world, int material, const Vector& pos, const Vector& size)
    : Body(world, PlayerBody), m_radius(size * 0.5f), m_isOnGround(true), m_force(), m_angleY(-M_PI/2)
{
    Matrix location = Matrix::rotateY(m_angleY) * Matrix::translate(pos);

    NewtonCollision* collision = NewtonCreateSphere(m_world, m_radius.x, m_radius.y, m_radius.z, NULL); 
    Body::create(collision, location);

	// disable auto freeze
	NewtonBodySetAutoFreeze(m_body, 0);
	NewtonWorldUnfreezeBody(m_world, m_body);

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
}

Player::~Player()
{
    NewtonDestroyJoint(m_world, m_upVector);
}

void Player::setForce(const Vector& force)
{
    m_force = force;
}

void Player::onRender(const Video* video) const
{
    glColor3f(0.5f, 0.5f, 0.5f);
    glScalef(m_radius.x, m_radius.y, m_radius.z);
    video->renderSphere(1.0f);
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
      
    Vector targetVel = m_force;
    targetVel.norm();
    if (!m_isOnGround)
    {
       NewtonBodyAddForce(m_body, force.v);
    }

    force = ( ( Matrix::rotateY(m_angleY)*targetVel*5.0f - currentVel ) * timestepInv ) * mass;
    if (!m_isOnGround) force.y = 0.0f;

    NewtonBodyAddForce(m_body, force.v);

    m_isOnGround = false;
  
/*
	// TODO: this is for rotation

    Vector omega, alpha;

    // calculate the torque vector
    float steerAngle = std::min (std::max ((_matrix.row(0) * cameraDir).y, -1.0f), 1.0f);
	steerAngle = dAsin (steerAngle); 
	NewtonBodyGetOmega(m_myBody, &omega.m_x);

	dVector torque (0.0f, 0.5f * Iyy * (steerAngle * timestepInv - omega.m_y) * timestepInv, 0.0f);
	NewtonBodySetTorque (m_myBody, &torque.m_x);
*/
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
