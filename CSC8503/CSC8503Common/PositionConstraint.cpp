#include "PositionConstraint.h"
#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace CSC8503;

PositionConstraint::PositionConstraint(GameObject* a, GameObject* b, float d)
{
	objectA = a;
	objectB = b;
	distance = d;
}

PositionConstraint::~PositionConstraint()
{

}

//a simple constraint that stops objects from being more than <distance> away
//from each other...this would be all we need to simulate a rope, or a ragdoll
void PositionConstraint::UpdateConstraint(float dt) {
	Vector3 relativePos =
		objectA->GetConstTransform().GetWorldPosition() -
		objectB->GetConstTransform().GetWorldPosition();

	float currentDistance = relativePos.Length();

	float offset = distance - currentDistance;

	if (abs(offset) > 0.0f) {
		Vector3 offsetDir = relativePos.Normalised();

		PhysicsObject* physA = objectA->GetPhysicsObject();
		PhysicsObject* physB = objectB->GetPhysicsObject();

		Vector3 relativeVelocity = physA->GetLinearVelocity() -
			physB->GetLinearVelocity();

		float constraintMass = physA->GetInverseMass() +
			physB->GetInverseMass();

		if (constraintMass > 0.0f) {
			// how much of their relative force is affecting the constraint
			float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);

			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 aImpulse = offsetDir * lambda;
			Vector3 bImpulse = -offsetDir * lambda;

			physA->ApplyLinearImpulse(aImpulse); // multiplied by mass here
			physB->ApplyLinearImpulse(bImpulse); // multiplied by mass here
		}
	}
}