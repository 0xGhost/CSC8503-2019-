#include "AppleObject.h"
#include "..\CSC8503Common\GameWorld.h"

void NCL::CSC8503::AppleObject::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetTag() == PlayerTag)
	{
		goose = (GooseObject*)otherObject;
		if (goose->CheckApple() == false)
		{
			goose->GetApple(this);
			float distance = (this->transform.GetWorldPosition() - goose->GetTransform().GetWorldPosition()).Length();

			constraint = new PositionConstraint(this, otherObject, distance);
			world->AddConstraint(constraint);
		}
	}
	else if(otherObject->GetTag() == HumanTag || otherObject->GetTag() == BallTag)
	{
		RemoveConstraint();
		goose->LostApple();
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
