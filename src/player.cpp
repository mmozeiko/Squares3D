#include "player.h"
#include "video.h"

#include "body.h" // TODO: remove

#include <cmath>
#include <GL/glfw.h>

Player::Player(const NewtonWorld* world, int material, const Vector& pos, const Vector& size)
    : Body(world), _radius(size * 0.5f), _stopped(true), _maxStepHigh(_radius.y * 0.5f), _force(),
    _angleY(-M_PI/2)
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
    _stopped = (force.x==0.0f && force.z==0.0f);
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

	// get the character mass
    float mass;
    Vector massI;
	NewtonBodyGetMassMatrix(_body, &mass, &massI.x, &massI.y, &massI.z);

	// apply the gravity force, cheat a little with the character gravity
	Vector force = mass * gravityVec;

	// Get the velocity vector
    Vector velocity;
	NewtonBodyGetVelocity(_body, velocity.v);

	// rotate the force direction to align with object angle
    Vector heading = Matrix::rotateY(_angleY) * _force;

    heading.norm();
    velocity.norm();

	force += (heading * 100.0f - heading * (50.0f * (velocity % heading)));

	NewtonBodySetForce(_body, force.v);

	// TODO: this is for rotation

/*
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
	Vector point;
	Vector normal;
	Vector velocity;

    // Get the collision and normal
	NewtonMaterialGetContactPositionAndNormal(material, point.v, normal.v);

	Vector localPoint(point * _matrix);

	// if a contact is below the max need consider the character is on the ground
	if (localPoint.y < _maxStepHigh)
    {
		NewtonBodyGetVelocity(_body, velocity.v);

		// calculate ball velocity perpendicular to the contact normal
		Vector tangentVelocity(velocity - normal * (normal % velocity));

		// align the tangent at the contact point with the tangent velocity vector of the ball
		NewtonMaterialContactRotateTangentDirections(material, tangentVelocity.v);

		// we do do want bound back we hitting the floor
		NewtonMaterialSetContactElasticity(material, 0.3f);
	
		// if the player want to move set disable friction else set high ground friction so it can stop on slopes
		if (_stopped)
        {
			NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 0);
			NewtonMaterialSetContactKineticFrictionCoef (material, 2.0f, 0);
			NewtonMaterialSetContactStaticFrictionCoef (material, 2.0f, 1);
			NewtonMaterialSetContactKineticFrictionCoef (material, 2.0f, 1);

		}
        else
        {
			NewtonMaterialSetContactFrictionState (material, 0, 0);
			NewtonMaterialSetContactFrictionState (material, 0, 1);
		}

	}
    else
    {
		//set contact above the max step to be friction less
		// disable fiction calculation for sphere collision
		NewtonMaterialSetContactFrictionState (material, 0, 0);
		NewtonMaterialSetContactFrictionState (material, 0, 1);
	}
}
