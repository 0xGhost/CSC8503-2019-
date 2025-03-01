#include "HumanObject.h"

//vector<GameObject*> emptyVector;
GameObjectIterator HumanObject::firstPlayer;// = emptyVector.end();
GameObjectIterator HumanObject::lastPlayer;// = emptyVector.end();

void NCL::CSC8503::HumanObject::UpdateDistance()
{
	distance = -1;
	for (auto& i = HumanObject::firstPlayer; i != HumanObject::lastPlayer; i++)
	{
		float tempDis = ((*i)->GetTransform().GetWorldPosition() - GetTransform().GetWorldPosition()).Length();
		if (distance > tempDis)
		{
			distance = tempDis;
			focusPlayer = *i;
		}
	}
}
