#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;
unsigned long long GameObject::nextId = 0;
GameObject::GameObject(string objectName, Tag objTag)	{
	name			= objectName;
	tag				= objTag;
	isActive		= true;
	isStatic		= false;
	layer			= 1;
	id				= nextId++;
	objTime = 0;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
	updateFunc = std::function<void(float, GameObject*)>([](float, GameObject*) {});
}

GameObject::~GameObject()	{
	delete physicsObject;
	delete renderObject;
	delete networkObject;
	delete boundingVolume;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

//These would be better as a virtual 'ToAABB' type function, really...
void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetWorldOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}