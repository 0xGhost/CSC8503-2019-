#include "HumanObject.h"

GameObjectIterator HumanObject::firstPlayer;
GameObjectIterator HumanObject::lastPlayer;
PhysicsSystem* HumanObject::physics;

void NCL::CSC8503::HumanObject::UpdateDistance()
{
	distance = 10000000000.0f;
	for (auto i = HumanObject::firstPlayer; i != HumanObject::lastPlayer; i++)
	{
		float tempDis = ((*i)->GetTransform().GetWorldPosition() - GetTransform().GetWorldPosition()).Length();
		if (distance > tempDis)
		{
			distance = tempDis;
			focusPlayer = *i;
		}
	}
}
