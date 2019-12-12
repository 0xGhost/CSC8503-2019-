#include "FrontPositionConstraint.h"
#include "../CSC8503Common/GameObject.h"

void NCL::CSC8503::FrontPositionConstraint::UpdateConstraint(float dt)
{
	PositionConstraint::UpdateConstraint(dt);
	Vector3 relativePos =
		objectA->GetConstTransform().GetWorldPosition() -
		objectB->GetConstTransform().GetWorldPosition();

	objectB->GetPhysicsObject()->ApplyLinearImpulse((relativePos.Normalised() - objectA->GetConstTransform().GetForward()) * 0.2f);
}
