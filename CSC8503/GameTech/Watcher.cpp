#include "WatcherObject.h"

WatcherObject::WatcherObject(string name, string tag) :HumanObject(name, tag) 
{
	attackDistance = 50;
	detectionDistance = 30;

	stateMachine = new StateMachine();

	StateFunc idleFunc = [&](void* data)
	{
		//Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		//Vector3 playerPos = (*Human::firstPlayer)->GetTransform().GetWorldPosition();
		HumanObject::UpdateDistance();
		this->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
	};

	StateFunc attackFunc = [&](void* data)
	{
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		float distance = (watcherPos - playerPos).Length();
		playerPos = playerPos + focusPlayer->GetPhysicsObject()->GetLinearVelocity() * distance / 10;
		Debug::DrawLine(watcherPos, playerPos);
		// TODO: throw a ball
	};

	GenericState* idleState = new GenericState(idleFunc, (void*)&distance);
	GenericState* attackState = new GenericState(attackFunc, (void*)&distance);

	stateMachine->AddState(idleState);
	stateMachine->AddState(attackState);

	GenericTransition <float&, float&>* transitionSeePlayer =
		new GenericTransition <float&, float&>(
			GenericTransition <float&, float&>::LessThanTransition,//GreaterThanTransition,
			distance, detectionDistance, idleState, attackState); // if greater than 10 , A to B

	GenericTransition <float&, float&>* transitionLostPlayer =
		new GenericTransition <float&, float&>(
			GenericTransition <float&, float&>::GreaterThanTransition,//GreaterThanTransition,
			distance, attackDistance, attackState, idleState); // if greater than 10 , A to B


	stateMachine->AddTransition(transitionSeePlayer);
	stateMachine->AddTransition(transitionLostPlayer);


}

void WatcherObject::Update(float dt)
{
	stateMachine->Update();
}
