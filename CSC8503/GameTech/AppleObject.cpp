#include "AppleObject.h"
#include "..\CSC8503Common\GameWorld.h"

void NCL::CSC8503::AppleObject::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetTag() == PlayerTag)
	{
		goose = (GooseObject*)otherObject;
		if (goose->CheckApple() == false)
		{
			goose->PickApple(this);
			float distance = (this->transform.GetWorldPosition() - goose->GetTransform().GetWorldPosition()).Length();

			constraint = new PositionConstraint(this, otherObject, distance);
			world->AddConstraint(constraint);
		}
	}
	else if(goose && otherObject->GetTag() == HumanTag || otherObject->GetTag() == BallTag)
	{
		RemoveConstraint();
		goose->LostApple();
		goose = nullptr;
	}
	else if (otherObject->GetTag() == HomeTag && goose)
	{
		RemoveConstraint();
		goose->BackHomeWithApple();
		transform.SetWorldPosition(originalPosition);
		physicsObject->ClearForces();
		physicsObject->SetLinearVelocity(Vector3(0, 0, 0));
		physicsObject->SetAngularVelocity(Vector3(0, 0, 0));
	}
}

void NCL::CSC8503::AppleObject::RemoveConstraint()
{
	if (constraint)
	{
		world->RemoveConstraint(constraint);
		delete constraint;
		constraint = nullptr;
	}
}
