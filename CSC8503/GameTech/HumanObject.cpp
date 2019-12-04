#include "HumanObject.h"

//vector<GameObject*> emptyVector;
GameObjectIterator HumanObject::firstPlayer;// = emptyVector.end();
GameObjectIterator HumanObject::lastPlayer;// = emptyVector.end();
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
