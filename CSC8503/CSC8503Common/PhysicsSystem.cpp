#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "../../Common/Quaternion.h"

#include "Constraint.h"
#include "QuadTree.h"
#include "Debug.h"

#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g) {
	applyGravity = false;
	useBroadPhase = false;
	dTOffset = 0.0f;
	globalDamping = 0.95f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
	worldSize = Vector3(1024, 1000, 1024);
	//quadTree = new QuadTree<GameObject*>(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 6);
	//staticQuadTree = new QuadTree<GameObject*>(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 6);
}

PhysicsSystem::~PhysicsSystem() {
	delete quadTree;
	delete staticQuadTree;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/
void PhysicsSystem::Update(float dt) {
	GameTimer testTimer;
	testTimer.GetTimeDeltaSeconds();

	frameDT = dt;

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	float iterationDt = 1.0f / 120.0f; //Ideally we'll have 120 physics updates a second 

	if (dTOffset > 8 * iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 15.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting physics iterations to 15" << iterationDt << std::endl;
	}
	else if (dTOffset > 4 * iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 30.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting iteration dt to 4 case " << iterationDt << std::endl;
	}
	else if (dTOffset > 2 * iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 60.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting iteration dt to 2 case " << iterationDt << std::endl;
	}
	else {
		//std::cout << "Running normal update " << iterationDt << std::endl;
	}

	int constraintIterationCount = 10;
	iterationDt = dt;

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}

	while (dTOffset > iterationDt * 0.5) {
		IntegrateAccel(iterationDt); //Update accelerations from external forces
		if (useBroadPhase) {
			UpdateQuadTree();
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = iterationDt / (float)constraintIterationCount;

		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);
		}

		IntegrateVelocity(iterationDt); //update positions from new velocity changes

		dTOffset -= iterationDt;
	}
	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions
	//std::cout << iteratorCount << " , " << iterationDt << std::endl;
	float time = testTimer.GetTimeDeltaSeconds();
	//std::cout << "Physics time taken: " << time << std::endl;
}

void NCL::CSC8503::PhysicsSystem::InitQuadTree()
{
	quadTree = new QuadTree<GameObject*>(Vector2(worldSize.x + 1, worldSize.z + 1), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 3);
	staticQuadTree = new QuadTree<GameObject*>(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 9);
	UpdateObjectAABBs();

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes) || (*i)->GetPhysicsObject()->GetInverseMass() != 0) {
			continue;
		}
		Vector3 pos = (*i)->GetConstTransform().GetWorldPosition();
		staticQuadTree->Insert(*i, pos, halfSizes);
	}
	useBroadPhase = true;
}

void NCL::CSC8503::PhysicsSystem::UpdateQuadTree()
{
	//delete quadTree;
	//quadTree = new QuadTree<GameObject*>(Vector2(512, 512), 6);
	//quadTree = new QuadTree<GameObject*>(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 6);

	quadTree->Update();

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes) || (*i)->GetPhysicsObject()->GetInverseMass() == 0) {
			continue;
		}
		Vector3 pos = (*i)->GetConstTransform().GetWorldPosition();
		quadTree->Insert(*i, pos, halfSizes);
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}
		(*i).framesLeft = (*i).framesLeft - 1;
		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if (!CheckLayerCollision((*i)->GetLayer(), (*j)->GetLayer()) || (*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				//std::cout << " Collision between " << (*i)->GetName() << " and " << (*j)->GetName() << std::endl;
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
		}
	}
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out.

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();
	if (totalMass == 0.0f) {
		return;
	}
	// Separate them out using projection
	transformA.SetWorldPosition(transformA.GetWorldPosition() -
		(p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));

	transformB.SetWorldPosition(transformB.GetWorldPosition() +
		(p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA =
		Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB =
		Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;


	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	if (Vector3::Dot(contactVelocity, p.normal) > 0) {
		return;
	}

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	// now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() *
		Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() *
		Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = physA->GetElasticity() * physB->GetElasticity(); // disperse some kinectic energy

	float j = (-(1.0f + cRestitution) * impulseForce) /
		(totalMass + angularEffect);

	Vector3 fullImpulse = p.normal * j;

	//physA->SetLinearVelocity(Vector3(0, 0, 0));
	//physB->SetLinearVelocity(Vector3(0, 0, 0));

	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	//j = (-(1.0f + cRestitution) * impulseForce) /
		//(totalMass + angularEffect);

	//fullImpulse = p.normal * j;

	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));
}

void NCL::CSC8503::PhysicsSystem::ResolveSpringCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const
{
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();
	// TODO
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to.

*/

void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	//QuadTree<GameObject*> tree(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 6);
	//gameWorld.tree(Vector2(worldSize.x, worldSize.z), (int)(log2((worldSize.x + worldSize.z) / 2.0f)), 6);

	//std::vector<GameObject*>::const_iterator first;
	//std::vector<GameObject*>::const_iterator last;
	//gameWorld.GetObjectIterators(first, last);
	//for (auto i = first; i != last; ++i) {
	//	Vector3 halfSizes;
	//	if (!(*i)->GetBroadphaseAABB(halfSizes)) {
	//		continue;
	//	}
	//	Vector3 pos = (*i)->GetConstTransform().GetWorldPosition();
	//	tree.Insert(*i, pos, halfSizes);
	//}
	int dymCount = 0;
	quadTree->OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) {
		CollisionDetection::CollisionInfo info;

		for (auto i = data.begin(); i != data.end(); ++i) {
			dymCount++;
			for (auto j = std::next(i); j != data.end(); ++j) {
				// is this pair of items already in the collision set -
				// if the same pair is in another quadtree node together etc
				info.a = min((*i).object, (*j).object);
				info.b = max((*i).object, (*j).object);
				if (CheckLayerCollision(info.a->GetLayer(), info.b->GetLayer()))
					broadphaseCollisions.insert(info);
			}
			
			std::list<GameObject*> list = staticQuadTree->EntryIntersectionList(*i);
			for (auto j : list)
			{
				info.a = min((*i).object, j);
				info.b = max((*i).object, j);
				if (CheckLayerCollision(info.a->GetLayer(), info.b->GetLayer()))
					broadphaseCollisions.insert(info);
			}
		}
	});
	if(quadTreeDebug)
		quadTree->DebugDraw(Vector4(0.9f,0.2f,0.5f,1));
	if(staticTreeDebug)
		staticQuadTree->DebugDraw(Vector4(0.7f, 0.5f, 0.2f, 1));
}

/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator
		i = broadphaseCollisions.begin();
		i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		if (CheckLayerCollision(info.a->GetLayer(), info.b->GetLayer())
			&& CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			if(!info.a->GetPhysicsObject()->IsTrigger() && !info.b->GetPhysicsObject()->IsTrigger())
				ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info); // insert into our main set
		}
	}

}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc.

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	GameObjectIterator first;
	GameObjectIterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue; // No physics object for this GameObject !
		}
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (applyGravity && inverseMass > 0) {
			accel += gravity / powf(1.0f - object->GetFriction(), dt); // don �t move infinitely heavy things
		}

		linearVel += accel * dt; // integrate accel !
		object->SetLinearVelocity(linearVel);

		// Angular stuff
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor(); // update tensor vs orientation

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * dt; // integrate angular accel !
		object->SetAngularVelocity(angVel);
	}
}
/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt, bool useFriction) {
	GameObjectIterator first;
	GameObjectIterator last;
	gameWorld.GetObjectIterators(first, last);
	//float dampingFactor = 1.0f - 0.95f;
	//float frameDamping = powf(dampingFactor, dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		float frameDamping = powf(1.0f - object->GetFriction(), dt);

		Transform& transform = (*i)->GetTransform();
		// Position Stuff
		Vector3 position = transform.GetLocalPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		//transform.SetLocalPosition(position);
		transform.SetLocalPosition(position);
		transform.SetWorldPosition(position);

		// Linear Damping
		linearVel = linearVel * (useFriction? frameDamping : 1);
		object->SetLinearVelocity(linearVel);

		// Orientation Stuff
		Quaternion orientation = transform.GetLocalOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		orientation = orientation +
			(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();

		transform.SetLocalOrientation(orientation);



		// Damp the angular velocity too
		angVel = angVel * frameDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		//Clear our object's forces for the next frame
		(*i)->GetPhysicsObject()->ClearForces();
	}
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc.

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}

bool PhysicsSystem::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject, unsigned int layerMask) const {
	//The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;

	std::list<GameObject*> list = quadTree->RayCastList(r);
	std::list<GameObject*> list1 = staticQuadTree->RayCastList(r);
	list.insert(list.end(), list1.begin(), list1.end());
	
	for (auto i : list) {
		if (!i->GetBoundingVolume() || ((1 << i->GetLayer()) & layerMask) == 0) { //objects might not be collideable etc... // add: layerMask
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
			if (!closestObject) {
				closestCollision = collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	if (collision.node) {
		closestCollision = collision;
		closestCollision.node = collision.node;
		return true;
	}

	return false;
}