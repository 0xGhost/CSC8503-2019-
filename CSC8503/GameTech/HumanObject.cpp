#include "HumanObject.h"

PlayerIterator HumanObject::firstPlayer;
PlayerIterator HumanObject::lastPlayer;
PhysicsSystem* HumanObject::physics;
int HumanObject::rayCastLayerMask = ~((1 << 3) | (1 << 6) | (1 << 7));

NCL::CSC8503::HumanObject::HumanObject(string n, Tag t) : GameObject(n, t)
{
	focusPlayer = nullptr;
	waterFriciton = 0.999f;
	groundFriction = 0.95f;
}

void NCL::CSC8503::HumanObject::UpdateDistance()
{
	focusPlayer = *HumanObject::firstPlayer;
	/*distance = 10000000000.0f;
	for (auto i = HumanObject::firstPlayer; i != HumanObject::lastPlayer; i++)
	{
		float tempDis = ((*i)->GetTransform().GetWorldPosition() - GetTransform().GetWorldPosition()).Length();
		if (distance > tempDis)
		{
			distance = tempDis;
			focusPlayer = (GooseObject*)* i;
		}
	}*/
}

void NCL::CSC8503::HumanObject::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetTag() == WaterTag)
	{
		physicsObject->SetFriction(waterFriciton);
	}
	else if (otherObject->GetTag() == TileTag)
	{
		physicsObject->SetFriction(groundFriction);
	}
}
