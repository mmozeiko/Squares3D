#include "player.h"
#include "video.h"

#include "body.h" // TODO: remove

#include <cmath>
#include <GL/glfw.h>

Player::Player(const NewtonWorld* world, int material, const Vector& pos, const Vector& size)
    : Body(world, PlayerBody), _radius(size * 0.5f), _isOnGround(true), _force(), _angleY(-M_PI/2)
{
    Matrix location = Matrix::rotateY(_angleY) * Matrix::translate(pos);

    NewtonCollision* collision = NewtonCreateSphere(_world, _radius.x, _radius.y, _radius.z, NULL); 
    Body::Create(collision, location);

	// disable auto freeze
	NewtonBodySetAutoFreeze(_body, 0);
	NewtonWorldUnfreezeBody(_world, _body);

	// set the viscous damping the the minimum
    const float damp[] = { 0.0f, 0.0f, 0.0f };
	NewtonBodySetLinearDamping(_body, 0.0f);
	NewtonBodySetAngularDamping(_body, damp);

	// Set Material Id for this object
	NewtonBodySetMaterialGroupID(_body, material);

	// set the mas
    Vector intertia, origin;
    NewtonConvexCollisionCalculateInertialMatrix(collision, intertia.v, origin.v);
    NewtonBodySetMassMatrix(_body, 10.0f, intertia.x, intertia.y, intertia.z);

  	// add and up vector constraint to help in keeping the body upright
	const Vector upDirection (0.0f, 1.0f, 0.0f);

    _upVector = NewtonConstraintCreateUpVector(_world, upDirection.v, _body); 
}

Player::~Player()
{
    NewtonDestroyJoint(_world, _upVector);
}

void Player::SetForce(const Vector& force)
{
    _force = force;
}

void Player::onRender(const Video& video) const
{
    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(_radius.x, _radius.y, _radius.z);
    video.RenderSphere(1.0f);
}

void Player::onSetForceAndTorque()
{
	float timestepInv = 1.0f / 0.01f; // 0.01f == DT

    float mass;
    float Ixx, Iyy, Izz;

    // Get the mass of the object
    NewtonBodyGetMassMatrix(_body, &mass, &Ixx, &Iyy, &Izz );

    Vector force = gravityVec * mass;

    Vector currentVel;
    NewtonBodyGetVelocity(_body, currentVel.v);
      
    Vector targetVel = _force;
    if (!_isOnGround)
    {
       NewtonBodyAddForce(_body, force.v);
    }

    force = ( ( Matrix::rotateY(_angleY)*targetVel - currentVel ) * timestepInv ) * mass;
    if (!_isOnGround) force.y = 0.0f;

    NewtonBodyAddForce(_body, force.v);

    _isOnGround = false;
  
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
   _isOnGround = (angle > 0.0f);


   NewtonMaterialSetContactElasticity( material, 0.0f );

   NewtonMaterialSetContactFrictionState( material, 0, 0 );
   NewtonMaterialSetContactFrictionState( material, 0, 1 );

   NewtonMaterialSetContactSoftness( material, 0 );

}
