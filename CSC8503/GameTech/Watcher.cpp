#include "Watcher.h"

NCL::CSC8503::Watcher::Watcher(string name, string tag) :Human(name, tag) 
{
	stateMachine = new StateMachine();

	StateFunc idleFunc = [&](void* data)
	{
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = firstPlayer->GetTransform().GetWorldPosition();
		
		this->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
	};

	StateFunc attackFunc = [&](void* data)
	{
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = firstPlayer->GetTransform().GetWorldPosition();
		float distance = (watcherPos - playerPos).Length();
		playerPos = playerPos + firstPlayer->GetPhysicsObject()->GetLinearVelocity() * distance / 10;
		Debug::DrawLine(watcherPos, playerPos);
		// TODO: throw a ball
	};

	GenericState* idleState = new GenericState(idleFunc, (void*)&distance);
	GenericState* attackState = new GenericState(attackFunc, (void*)&distance);

	stateMachine->AddState(idleState);
	stateMachine->AddState(attackState);

}

void NCL::CSC8503::Watcher::Update(float dt)
{
	stateMachine->Update();
}
