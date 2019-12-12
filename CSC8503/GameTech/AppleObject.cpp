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

			constraint = new FrontPositionConstraint(otherObject, this, distance); // TODO new Constraint help the apple stay at front
			world->AddConstraint(constraint);
		}
	}
	else if (goose && (otherObject->GetTag() == HumanTag || otherObject->GetTag() == BallTag))
	{
		RemoveConstraint();
		goose->LostApple();
		goose = nullptr;
	}
	else if (otherObject->GetTag() == HomeTag && goose)
	{
		RemoveConstraint();
		goose->BackHomeWithApple(score);
		goose = nullptr;
		//transform.SetWorldPosition(originalPosition);
		transform.SetWorldPosition(transform.GetWorldPosition() + Vector3(0,-100,0));
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
