#include "Physics.h"

Physics::Physics(btVector3 gravity)
{
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(gravity);
}

glm::vec3 Physics::toGlm(btVector3 vector3) {
	return glm::vec3(vector3.getX(), vector3.getY(), vector3.getZ());
}

btVector3 Physics::fromGlm(glm::vec3 vector3) {
	return btVector3(vector3.x, vector3.y, vector3.z);
}

glm::quat Physics::toGlm(btQuaternion rot) {
	return glm::quat(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
}

btQuaternion Physics::fromGlm(glm::quat rot) {
	return btQuaternion(rot.x, rot.y, rot.z, rot.w);
}

void Physics::addGhostSphere(btVector3 position, btScalar scale) {
	ghost = new btGhostObject();
	ghost->setCollisionShape(new btBoxShape(btVector3(scale, scale, scale)));
	btTransform startTransform;
	startTransform.setIdentity();
	ghost->setWorldTransform(startTransform);
	dynamicsWorld->addCollisionObject(ghost);
	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}

void Physics::addGround(btVector3 position, btVector3 scale)
{
	btCollisionShape* groundShape = new btBoxShape(scale);

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(position);

	btScalar mass(0.);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static

	btVector3 localInertia(0, 0, 0);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	dynamicsWorld->addRigidBody(body);
}

void Physics::explode(btVector3 position, btScalar strength, btScalar decay, btCollisionObject* obj) {
	for (int i = 0; i < dynamicsWorld->getNumCollisionObjects() - 1; ++i) {
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i + 1];
		btRigidBody *body = btRigidBody::upcast(obj);
		btVector3 diff = body->getWorldTransform().getOrigin() - position;
		btScalar dis = diff.norm();
		btScalar power = strength - decay * dis;
		if (power > 0) {
			body->applyImpulse(power * diff, btVector3(0, 0, 0));
		}
	}
	dynamicsWorld->removeCollisionObject(obj);
}

btRigidBody* Physics::addSphere(btVector3 position, float scale)
{
	//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
	btCollisionShape* colShape = new btSphereShape(scale);
	collisionShapes.push_back(colShape);

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static

	btVector3 localInertia(0, 0, 0);

	colShape->calculateLocalInertia(mass, localInertia);

	startTransform.setOrigin(position);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	dynamicsWorld->addRigidBody(body);
	return body;
}

btRigidBody* Physics::addCube(btVector3 position, btVector3 half_ext)
{
	//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
	btCollisionShape* colShape = new btBoxShape(half_ext);
	collisionShapes.push_back(colShape);

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass(1.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	 
	btVector3 localInertia(0, 0, 0);

	colShape->calculateLocalInertia(mass, localInertia);

	startTransform.setOrigin(position);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	body->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(body);
	return body;
}

void Physics::deleteObject(btCollisionObject* obj) {
	//btCollisionObject* to_delete = getObject(index);
	dynamicsWorld->removeCollisionObject(obj);
}

void Physics::step(float deltaTime) {
	dynamicsWorld->stepSimulation(deltaTime, 1, 1.0f / 90.0f);
}

btCollisionObject* Physics::getObject(int index) {
	return dynamicsWorld->getCollisionObjectArray()[index];
}

Physics::~Physics()
{
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynamicsWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
}
