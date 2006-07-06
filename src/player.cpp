#include "player.h"
#include "video.h"

#include <cmath>
#include <GL/glfw.h>

#define GRAVITY     -9.81f

Player::Player(const NewtonWorld* world, int material, const Vector& pos, const Vector& size) : _world(world)
{
    // TODO: move to common physics object parent class
	_stopped = true;

	// calculate the character ellipse radius
	_radius = size * 0.5f;

    _maxStepHigh = _radius.y * 0.5f;

    Matrix location = Matrix::translate(pos);
    location *= Matrix::rotateY(M_PI/2);

	NewtonCollision* sphere = NewtonCreateSphere(_world, _radius.x, _radius.y, _radius.z, NULL); 
	_body = NewtonCreateBody(_world, sphere);
	NewtonReleaseCollision(_world, sphere);

	// disable auto freeze management for the player
	NewtonBodySetAutoFreeze(_body, 0);

	// keep the player always active 
	NewtonWorldUnfreezeBody(_world, _body);

    const float damp[] = { 0.0f, 0.0f, 0.0f };
	// set the viscous damping the the minimum
	NewtonBodySetLinearDamping(_body, 0.0f);
	NewtonBodySetAngularDamping(_body, damp);

	// Set Material Id for this object
	NewtonBodySetMaterialGroupID(_body, material);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData(_body, static_cast<void*>(this));

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback(_body, OnApplyForce);

	// set the mas
	NewtonBodySetMassMatrix(_body, 10.0f, 10.0f, 10.0f, 10.0f);

	// set the matrix for the rigid body
    location.transpose();
	NewtonBodySetMatrix(_body, location.m);

  	// add and up vector constraint to help in keeping the body upright
	const Vector upDirection (0.0f, 1.0f, 0.0f);

    _upVector = NewtonConstraintCreateUpVector(_world, upDirection.v, _body); 
}

Player::~Player()
{
    NewtonDestroyJoint(_world, _upVector);
    NewtonDestroyBody(_world, _body);
}

void Player::setForce(const Vector& force)
{
    _force = force;
    _stopped = (force.x==0.0f && force.z==0.0f);
}

void Player::prepare()
{
    NewtonBodyGetMatrix(_body, _matrix.m);
    //_matrix.transpose();// - wtf, why not needed ???
}

void Player::render(const Video& video) const
{
    glPushMatrix();
    glMultMatrixf(_matrix.m);

    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(_radius.x, _radius.y, _radius.z);
    video.renderSphere(1.0f);

    glPopMatrix();
}

void Player::OnApplyForce(const NewtonBody* body)
{
    Player* self = static_cast<Player*>(NewtonBodyGetUserData(body));
    self->OnApplyForce();
}

void Player::OnApplyForce()
{
    float mass;
    Vector massI;

    Vector omega, alpha, velocity;

    Matrix matrix;

	float timestepInv = 1.0f / 0.01f; // 0.01f == DT

	// get the character mass
	NewtonBodyGetMassMatrix(_body, &mass, &massI.x, &massI.y, &massI.z);

	// apply the gravity force, cheat a little with the character gravity
	Vector force = Vector (0.0f, mass*GRAVITY, 0.0f);

	// Get the velocity vector
	NewtonBodyGetVelocity(_body, velocity.v);

	// determine if the character have to be snap to the ground
	NewtonBodyGetMatrix(_body, matrix.m);

	// rotate the force direction to align with the camera
	Vector heading = matrix * _force;
	heading.norm();

	force += (heading * 100.0f - heading * (50.0f * (velocity % heading)));
	NewtonBodySetForce(_body, force.v);

	// TODO: this is for rotation

/*
    // calculate the torque vector
    float steerAngle = std::min (std::max ((_matrix.row(0) * cameraDir).y, -1.0f), 1.0f);
	steerAngle = dAsin (steerAngle); 
	NewtonBodyGetOmega(m_myBody, &omega.m_x);

	dVector torque (0.0f, 0.5f * Iyy * (steerAngle * timestepInv - omega.m_y) * timestepInv, 0.0f);
	NewtonBodySetTorque (m_myBody, &torque.m_x);
*/
}

void Player::OnCollision(const NewtonMaterial* material, const NewtonContact* contact)
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
