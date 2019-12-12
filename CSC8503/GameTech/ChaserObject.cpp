#include "ChaserObject.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "GooseObject.h"



ChaserObject::ChaserObject(string name, Tag tag) :HumanObject(name, tag)
{
	searchDistance = 50;
	detectionDistance = 30;
	stateMachine = new StateMachine();
	state = 0;
	superState = 0;
	
	StateFunc idleFunc = [&]()
	{
		//int* state = (int*)data;
		//HumanObject::UpdateDistance();
		this->GetPhysicsObject()->AddTorque(Vector3(0, -50, 0));
		this->GetRenderObject()->SetColour(Vector4(0.3f, 0.9f, 0.9f, 1));
		
		Vector3 opos(originPosition.x, 0, originPosition.z);
		Vector3 ppos = this->GetTransform().GetWorldPosition();
		ppos.y = 0;
		float distance = (opos - ppos).Length();

		if (distance > 5.0f)
		{
			state = 3;
			return;
		}

		/*Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		Vector3 rayDir = playerPos - watcherPos;
		distance = (watcherPos - playerPos).Length();
		if (distance < detectionDistance)
		{
			RayCollision closestCollision;
			Ray ray(watcherPos, rayDir.Normalised());

			if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
			{
				if (((GameObject*)closestCollision.node)->GetTag() == PlayerTag
					&& ((GooseObject*)closestCollision.node)->CheckApple())
				{
					state = 1;
				}
			}
			Debug::DrawLine(watcherPos, playerPos);
		}*/
	};

	StateFunc returnFunc = [&]()
	{
		//int* state = (int*)data;
		this->GetRenderObject()->SetColour(Vector4(0.2f, 0.9f, 0.2f, 1));
		Vector3 opos(originPosition.x, 0, originPosition.z);
		Vector3 ppos = this->GetTransform().GetWorldPosition();
		ppos.y = 0;
		float distance = (opos - ppos).Length();

		if (distance < 0.2f)
		{
			state = 0;
			return;
		}

		//Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		//Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		//Vector3 direction = playerPos - watcherPos;

		//RayCollision closestCollision;
		//Ray ray(watcherPos, direction.Normalised());

		//if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
		//{
		//	if (((GameObject*)closestCollision.node)->GetTag() == PlayerTag
		//		&& ((GooseObject*)closestCollision.node)->CheckApple())
		//	{
		//		state = 1;
		//		return;
		//	}
		//}
		path.Clear();
		getPath(originPosition, this);

		Vector3 nextPos;
		path.PopWaypoint(nextPos); path.PopWaypoint(nextPos);
		nextPos += worldOffset;
		Vector3 moveDir = nextPos - this->GetTransform().GetWorldPosition();
		moveDir.y = 0;
		Movement(moveDir, walkForce);

		//Vector3 pos;
		//vector<Vector3> testNodes;
		//while (path.PopWaypoint(pos)) {
		//	testNodes.push_back(pos);
		//}
		//for (int i = 1; i < testNodes.size(); ++i) {
		//	Vector3 a = testNodes[i - 1];
		//	Vector3 b = testNodes[i];
		//	a.y = 15;
		//	b.y = 15;
		//	a += worldOffset;
		//	b += worldOffset;
		//	Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		//}

	};

	StateFunc searchFunc = [&]()
	{
		this->GetRenderObject()->SetColour(Vector4(0.5f, 0.5f, 0.0f, 1));

		//int* state = (int*)data;
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		//float distance = (watcherPos - playerPos).Length();
		//if (distance > searchDistance || !focusPlayer->CheckApple())
		//{
		//	state = 3;
		//	return;
		//}

		Debug::DrawLine(watcherPos, playerPos, Vector4(0, 1, 1, 1));

		Vector3 direction = playerPos - watcherPos;

		RayCollision closestCollision;
		Ray ray(watcherPos, direction.Normalised());

		if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
		{
			if (((GameObject*)closestCollision.node)->GetTag() == PlayerTag
				&& ((GooseObject*)closestCollision.node)->CheckApple())
			{
				state = 1;
			}
		}
		path.Clear();
		getPath(playerPos, this);

		/*Vector3 pos;
		vector<Vector3> testNodes;
		while (path.PopWaypoint(pos)) {
			testNodes.push_back(pos);
		}
		for (int i = 1; i < testNodes.size(); ++i) {
			Vector3 a = testNodes[i - 1];
			Vector3 b = testNodes[i];
			a.y = 15;
			b.y = 15;
			a += Vector3(-45, 0, -45);
			b += Vector3(-45, 0, -45);
			Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		}*/

		Vector3 nextPos;
		path.PopWaypoint(nextPos); path.PopWaypoint(nextPos);
		nextPos += worldOffset;
		Vector3 moveDir = nextPos - this->GetTransform().GetWorldPosition();
		moveDir.y = 0;
		Movement(moveDir, walkForce);

	};

	StateFunc chaseFunc = [&]()
	{
		this->GetRenderObject()->SetColour(Vector4(0.9f, 0.0f, 0.0f, 1));

		//int* state = (int*)data;
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();

		Debug::DrawLine(watcherPos, playerPos, Vector4(0, 1, 1, 1));

		Vector3 direction = playerPos - watcherPos;

		RayCollision closestCollision;
		Ray ray(watcherPos, direction.Normalised());

		if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
		{
			if (((GameObject*)closestCollision.node)->GetTag() != PlayerTag
				|| !((GooseObject*)closestCollision.node)->CheckApple())
			{
				state = 2;
			}
		}
		direction.y = 0;

		this->Movement(direction, runForce);
	};

	StateFunc defenseFunc = [&]()
	{
		HumanObject::UpdateDistance(); //  TODO: move to main update just do it once!
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		Vector3 rayDir = playerPos - watcherPos;
		distance = (watcherPos - playerPos).Length();
		if (distance < detectionDistance)
		{
			RayCollision closestCollision;
			Ray ray(watcherPos, rayDir.Normalised());

			if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
			{
				if (((GameObject*)closestCollision.node)->GetTag() == PlayerTag
					&& ((GooseObject*)closestCollision.node)->CheckApple())
				{
					superState = 1;
				}
			}
			Debug::DrawLine(watcherPos, playerPos);
		}
	};


	StateFunc offenseFunc = [&]()
	{
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		float distance = (watcherPos - playerPos).Length();
		if (distance > searchDistance || !focusPlayer->CheckApple())
		{
			superState = 0;
			state = 3;
		}
	};


	GenericState* idleState = new GenericState(idleFunc);
	GenericState* searchState = new GenericState(searchFunc);
	GenericState* chaseState = new GenericState(chaseFunc);
	GenericState* returnState = new GenericState(returnFunc);

	StateMachine* defenseSubStateMachine = new StateMachine();
	StateMachine* offenseSubStateMachine = new StateMachine();

	defenseSubStateMachine->AddState(returnState);
	defenseSubStateMachine->AddState(idleState);
	offenseSubStateMachine->AddState(chaseState);
	offenseSubStateMachine->AddState(searchState);



	SuperState* defenseSuperState = new SuperState(defenseFunc, defenseSubStateMachine);
	SuperState* offenseSuperState = new SuperState(offenseFunc, offenseSubStateMachine);

	stateMachine->AddState(defenseSuperState);
	stateMachine->AddState(offenseSuperState);


	/*stateMachine->AddState(idleState);
	stateMachine->AddState(searchState);
	stateMachine->AddState(chaseState);
	stateMachine->AddState(returnState);*/

	GenericTransition<int& , int>* superTransition0to1 = 
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			superState, 1, defenseSuperState, offenseSuperState);

	GenericTransition<int&, int>* superTransition1to0 =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			superState, 0, offenseSuperState, defenseSuperState);

	//GenericTransition <int&, int>* transition0to1 =
	//	new GenericTransition <int&, int>(
	//		GenericTransition <int&, int>::EqualsTransition,
	//		state, 1, idleState, chaseState);

	GenericTransition <int&, int>* transition1to2 =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			state, 2, chaseState, searchState);

	GenericTransition <int&, int>* transition2to1 =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			state, 1, searchState, chaseState);

	//GenericTransition <int&, int>* transition2to3 =
	//	new GenericTransition <int&, int>(
	//		GenericTransition <int&, int>::EqualsTransition,
	//		state, 3, searchState, returnState);

	GenericTransition <int&, int>* transition3to0 =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			state, 0, returnState, idleState);

	GenericTransition <int&, int>* transition0to3 =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			state, 3, idleState, returnState);

	//GenericTransition <int&, int>* transition3to1 =
	//	new GenericTransition <int&, int>(
	//		GenericTransition <int&, int>::EqualsTransition,
	//		state, 1, returnState, chaseState);

	stateMachine->AddTransition(superTransition0to1);
	stateMachine->AddTransition(superTransition1to0);

	defenseSubStateMachine->AddTransition(transition3to0);
	defenseSubStateMachine->AddTransition(transition0to3);
	offenseSubStateMachine->AddTransition(transition1to2);
	offenseSubStateMachine->AddTransition(transition2to1);

	/*stateMachine->AddTransition(transition0to1);
	stateMachine->AddTransition(transition1to2);
	stateMachine->AddTransition(transition2to1);
	stateMachine->AddTransition(transition2to3);
	stateMachine->AddTransition(transition3to0);
	stateMachine->AddTransition(transition3to1);*/



}

void NCL::CSC8503::ChaserObject::InitOriginPosition()
{
	originPosition = transform.GetWorldPosition();
}

void NCL::CSC8503::ChaserObject::Movement(Vector3 direction, float force)
{
	physicsObject->AddForceAtLocalPosition(direction.Normalised() * force, transform.GetForward() * 0.1);
}
