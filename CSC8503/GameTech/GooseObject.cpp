#include "GooseObject.h"
#include "AppleObject.h"

void NCL::CSC8503::GooseObject::Update(float dt)
{
	Vector3 force = transform.GetUp().Normalised() - Vector3(0, 1, 0);
	Vector3 torque = Vector3::Cross(Vector3(0, 1, 0), force);
	physicsObject->AddTorque(-torque * 20);
}

void NCL::CSC8503::GooseObject::OnCollisionBegin(GameObject* otherObject)
{
	if (apple && (otherObject->GetTag() == HumanTag || otherObject->GetTag() == BallTag))
	{
		apple->RemoveConstraint();
		apple = nullptr;
	}
}

