#include "player.h"
#include "video.h"

#include <cmath>
#include <GL/glfw.h>

#define GRAVITY     -9.81f
#define JUMP_TIMER	4

Player::Player(const NewtonWorld* world, int material, const Vector& pos, const Vector& size) : _world(world)
{
    // TODO: move to common physics object parent class

/*    _collision = NewtonCreateBox(_world, 1.0f, 1.0f, 1.0f, NULL);
    _body = NewtonCreateBody(_world, _collision);
    NewtonBodySetMassMatrix(_body, 1.0f, 1.0f, 1.0f, 1.0f);
    NewtonReleaseCollision(_world, _collision);

    Matrix matrix = Matrix::translate(pos);
    matrix.transpose();
    NewtonBodySetMatrix(_body, matrix.m);

    NewtonBodySetForceAndTorqueCallback(_body, OnApplyForce);
    NewtonBodySetMaterialGroupID(_body, material);

    NewtonBodySetAutoFreeze(_body, 0);

    NewtonBodySetUserData(_body, reinterpret_cast<void*>(this));
*/
    _force = Vector();

	// assume the character is on the air
	_inAir = true;
	_stopped = true;
	_jumpTimer = 0;

	_stepContact = Vector(0.0f, - _radius.y, 0.0f);   
	_maxStepHigh = - _radius.y * 0.5f;

	// calculate the character ellipse radius
	_radius = size * 0.5f;
    
	// scale the Max translational before considering tunneling prevention 
	_maxTranslation = size.x * 0.25f;

    Matrix location = Matrix::translate(Vector(pos.x, findFloor(pos, 100) + _radius.y, pos.z));
    location *= Matrix::rotateY(M_PI/2);

	// place a sphere at the center
	NewtonCollision* sphere = NewtonCreateSphere(_world, _radius .x, _radius .y, _radius .z, NULL); 

	// wrap the character collision under a transform, modifier for tunneling trught walls avoidance
	_collision = NewtonCreateConvexHullModifier (_world, sphere);
	NewtonReleaseCollision(_world, sphere);

	//create the rigid body
	_body = NewtonCreateBody(_world, _collision);

	// disable auto freeze management for the player
	NewtonBodySetAutoFreeze(_body, 0);

	// keep the player always active 
	NewtonWorldUnfreezeBody(_world, _body);

    float damp[] = { 0.0f, 0.0f, 0.0f };
	// set the viscous damping the the minimum
	NewtonBodySetLinearDamping(_body, 0.0f);
	NewtonBodySetAngularDamping(_body, damp);

	// Set Material Id for this object
	NewtonBodySetMaterialGroupID(_body, material);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData(_body, this);

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

	// release the collision geometry when not need it
	NewtonReleaseCollision(_world, _collision);
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
    if (force.y != 0.0f)
    {
        _jumpTimer = 4;
    }
}

void Player::prepare()
{
    NewtonBodyGetMatrix(_body, _matrix.m);
    //_matrix.transpose();// - wtf, why not needed ???
}

void Player::render(const Video& video)
{
    glPushMatrix();
    glMultMatrixf(_matrix.m);

    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(_radius.x, _radius.y, _radius.z);
    video.renderSphere(1.0f);

    glPopMatrix();
}

struct RayCast
{
    NewtonBody* _body;
    float _param;

    RayCast(NewtonBody* body)
    {
        _body = body;
        _param = 1.2f;
    }
};

float Player::findFloor(const Vector& centre, float maxDist)
{
    RayCast data(_body);

    Vector c(centre);
    c.y -= maxDist;

    NewtonWorldRayCast(_world, centre.v, c.v, OnRayCast, &data, NULL);
    
    return centre.y - maxDist * data._param;
}

float Player::OnRayCast(const NewtonBody* body, const float* normal, int collisionID, void* userData, float intersetParam)
{
    float paramPtr;
    RayCast* data = reinterpret_cast<RayCast*>(userData);

    paramPtr = 1.2f;
    if (data->_body != body)
    {
		if (intersetParam < data->_param) {
			data->_param = intersetParam;
			paramPtr = intersetParam;
		}
    }

    return paramPtr;
}

void Player::OnApplyForce(const NewtonBody* body)
{
    Player* self = reinterpret_cast<Player*>(NewtonBodyGetUserData(body));
    self->OnApplyForce();
}

void Player::OnApplyForce()
{
//    static const Vector gravity(0.0f, GRAVITY, 0.0f);
//    NewtonBodyAddForce(_body, gravity.v);
//    NewtonBodyAddForce(_body, _force.v);

    float mass;
    Vector massI;

    Vector omega, alpha, velocity;

    Matrix matrix;

	float timestep = NewtonGetTimeStep(_world);
	float timestepInv = 1.0f / timestep;

	// get the character mass
	NewtonBodyGetMassMatrix(_body, &mass, &massI.x, &massI.y, &massI.z);

	// apply the gravity force, cheat a little with the character gravity
	Vector force = Vector (0.0f, mass * GRAVITY, 0.0f);

	// Get the velocity vector
	NewtonBodyGetVelocity(_body, velocity.v);

	// determine if the character have to be snap to the ground
	NewtonBodyGetMatrix(_body, matrix.m);

	// if the floor is with in reach then the character must be snap to the ground
	// the max allow distance for snapping i 0.25 of a meter
    if (_inAir && !_jumpTimer)
    { 
		float floor = findFloor(matrix.row(3), _radius.y + 0.25f);
		float deltaHeight = (matrix.m31 - _radius.y) - floor;
		if ((deltaHeight < (0.25f - 0.001f)) && (deltaHeight > 0.01f))
        {
			// snap to floor only if the floor is lower than the character feet		
			float accelY = - (deltaHeight * timestepInv + velocity.y) * timestepInv;
			force.y = mass * accelY;
		}
	}
    else if (_jumpTimer == JUMP_TIMER)
    {
	   Vector veloc(0.0f, 7.5f, 0.0f);
	   NewtonAddBodyImpulse(_body, veloc.v, matrix.row(3).v);
	}

	_jumpTimer = _jumpTimer ? _jumpTimer - 1 : 0;
	
	// rotate the force direction to align with the camera
	Vector heading = matrix * _force;
	heading.norm();

	force += (heading * 30.0f - heading * (10.0f * velocity % heading)); 
	NewtonBodySetForce(_body, (force*2).v);

	// estimate the final horizontal translation for to next force and velocity
	Vector step( (force * (timestep  / mass) + velocity) * timestep);

	step = step * matrix;
	Matrix collisionPaddingMatrix;

	step.y = 0.0f;

	float dist = step % step;
	if (dist > _maxTranslation * _maxTranslation)
    {
		// when the velocity is high enough that can miss collision we will enlarge the collision 
		// long the vector velocity
        dist = std::sqrtf(dist);
		step /= dist;

		//make a rotation matrix that will align the velocity vector with the front vector
		collisionPaddingMatrix.m00 =  step[0];
		collisionPaddingMatrix.m02 = -step[2];
		collisionPaddingMatrix.m20 =  step[2];
		collisionPaddingMatrix.m22 =  step[0];

		// get the transpose of the matrix
		Matrix transp = collisionPaddingMatrix;
        transp.transpose();

        float f = dist/_maxTranslation;
		transp.m00 *= f;
		transp.m01 *= f;
		transp.m02 *= f;
		transp.m03 *= f;

		// calculate and oblique scale matrix by using a similar transformation matrix of the for, R'* S * R
		collisionPaddingMatrix = collisionPaddingMatrix * transp;
	}

	// set the collision modifierMatrix;
    collisionPaddingMatrix.transpose();
	NewtonConvexHullModifierSetMatrix (_collision, collisionPaddingMatrix.m);

	// TODO: this is for rotation

    // calculate the torque vector
//    float steerAngle = std::min (std::max ((_matrix.row(0) * cameraDir).y, -1.0f), 1.0f);
//	steerAngle = dAsin (steerAngle); 
//	NewtonBodyGetOmega(m_myBody, &omega.m_x);

//	dVector torque (0.0f, 0.5f * Iyy * (steerAngle * timestepInv - omega.m_y) * timestepInv, 0.0f);
//	NewtonBodySetTorque (m_myBody, &torque.m_x);

    // **

	// assume the character is on the air. this variable will be set to false if the contact detect 
	//the character is landed 
	_inAir = true;
	_stepContact = Vector(0.0f, -_radius.y, 0.0f);   

    const Vector pin(0.0f, 1.0f, 0.0f);
	NewtonUpVectorSetPin(_upVector, pin.v);
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
		_inAir = false;
			
		NewtonBodyGetVelocity(_body, velocity.v);

		// calculate ball velocity perpendicular to the contact normal
		Vector tangentVelocity(velocity - normal * (normal % velocity));

		// align the tangent at the contact point with the tangent velocity vector of the ball
		NewtonMaterialContactRotateTangentDirections(material, tangentVelocity.v);

		// we do do want bound back we hitting the floor
		NewtonMaterialSetContactElasticity(material, 0.3f);
	
		// save the elevation of the highest step to take
		if (localPoint.y > _stepContact.y)
        {
            if (std::fabs(normal.y) < 0.8f)
            {
				_stepContact = localPoint;   
			}
		}

		// if the player want to move set disable friction else set high ground friction so it can stop on slopes
		if (_stopped)
        {
			NewtonMaterialSetContactStaticFrictionCoef (material, 0.5f, 0);
			NewtonMaterialSetContactKineticFrictionCoef (material, 0.5f, 0);
			NewtonMaterialSetContactStaticFrictionCoef (material, 0.5f, 1);
			NewtonMaterialSetContactKineticFrictionCoef (material, 0.5f, 1);

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
