#include "GooseObject.h"
#include "AppleObject.h"

NCL::CSC8503::GooseObject::GooseObject(string n, Tag t) : GameObject(n, t) 
{ 
	apple = nullptr; 
	score = 0; 
	waterFriciton = 0.85f;
	groundFriction = 0.95f;
}


void NCL::CSC8503::GooseObject::Update(float dt)
{
	Vector3 force = transform.GetUp().Normalised() - Vector3(0, 1, 0);
	Vector3 torque = Vector3::Cross(Vector3(0, 1, 0), force);
	physicsObject->AddTorque(-torque * 20);
	Vector3 pos = transform.GetWorldPosition();
	if (pos.y < 0)
	{
		transform.SetWorldPosition(pos + Vector3(0, 10, 0));
	}
}

void NCL::CSC8503::GooseObject::OnCollisionBegin(GameObject* otherObject)
{
	if (apple && (otherObject->GetTag() == HumanTag || otherObject->GetTag() == BallTag))
	{
		apple->RemoveConstraint();
		apple = nullptr;
	}
	else if (otherObject->GetTag() == WaterTag)
	{
		physicsObject->SetFriction(waterFriciton);
	}
	else if (otherObject->GetTag() == TileTag)
	{
		physicsObject->SetFriction(groundFriction);
	}

}

