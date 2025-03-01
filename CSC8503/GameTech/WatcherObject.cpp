#include "WatcherObject.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "GooseObject.h"

WatcherObject::WatcherObject(string name, Tag tag) :HumanObject(name, tag) 
{
	attackDistance = 50;
	detectionDistance = 30;
	stateMachine = new StateMachine();

	StateFunc idleFunc = [&]()
	{
		//Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		//Vector3 playerPos = (*Human::firstPlayer)->GetTransform().GetWorldPosition();
		HumanObject::UpdateDistance();
		this->GetPhysicsObject()->AddTorque(Vector3(0, 50, 0));
		this->GetRenderObject()->SetColour(Vector4(0.3, 0.9, 0.9, 1));
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		distance = (watcherPos - playerPos).Length();
	};

	StateFunc aimFunc = [&]()
	{

		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		distance = (watcherPos - playerPos).Length();
		//Debug::DrawLine(watcherPos, playerPos, Vector4(0,1,1,1));
		Vector3 rayDir = playerPos - watcherPos;
		playerPos = playerPos + focusPlayer->GetPhysicsObject()->GetLinearVelocity() * distance / 70;
		Vector3 direction = playerPos - watcherPos;

		direction.y = 0;
		Vector3 torque = Vector3::Cross(this->GetTransform().GetForward(), -direction.Normalised());
		this->GetPhysicsObject()->AddTorque(torque * 100);

		RayCollision closestCollision;
		Ray ray(watcherPos, rayDir.Normalised());

		if (physics->Raycast(ray, closestCollision, true, rayCastLayerMask))
		{
			if (((GameObject*)closestCollision.node)->GetTag() == PlayerTag 
				&& ((GooseObject*)closestCollision.node)->CheckApple())
			{

				
				if (coolDownTime < 0.0f)
				{
					this->ready = 10;
				}

				this->GetRenderObject()->SetColour(Vector4(0.9f, 0.5f, 0.0f, 1));
			}
		}
		//Debug::DrawLine(watcherPos, playerPos);
		
	};

	StateFunc attackFunc = [&]()
	{
		Vector3 watcherPos = this->GetTransform().GetWorldPosition();
		Vector3 playerPos = focusPlayer->GetTransform().GetWorldPosition();
		playerPos = playerPos + focusPlayer->GetPhysicsObject()->GetLinearVelocity() * distance / 100;
		throwBall((playerPos - (watcherPos + Vector3(0,5,0))).Normalised(), this);
		coolDownTime = totalCoolDownTime;
		this->GetRenderObject()->SetColour(Vector4(0.9f, 0.0f, 0.1f, 1));
		ready = 5;
	};

	GenericState* idleState = new GenericState(idleFunc);
	GenericState* aimState = new GenericState(aimFunc);
	GenericState* attackState = new GenericState(attackFunc);


	stateMachine->AddState(idleState);
	stateMachine->AddState(aimState);
	stateMachine->AddState(attackState);

	GenericTransition <float&, float&>* transitionSeePlayer =
		new GenericTransition <float&, float&>(
			GenericTransition <float&, float&>::LessThanTransition,
			distance, detectionDistance, idleState, aimState); 

	GenericTransition <float&, float&>* transitionLostPlayer =
		new GenericTransition <float&, float&>(
			GenericTransition <float&, float&>::GreaterThanTransition,
			distance, attackDistance, aimState, idleState); 

	GenericTransition <int&, int>* transitionAttackPlayer =
		new GenericTransition <int&, int>(
			GenericTransition <int&, int>::EqualsTransition,
			ready, 10, aimState, attackState);

	GenericTransition <bool, bool>* transitionAttackFinish =
		new GenericTransition <bool, bool>(
			GenericTransition <bool, bool>::EqualsTransition,
			true, true, attackState, aimState);

	stateMachine->AddTransition(transitionSeePlayer);
	stateMachine->AddTransition(transitionLostPlayer);
	stateMachine->AddTransition(transitionAttackPlayer);
	stateMachine->AddTransition(transitionAttackFinish);


}


void WatcherObject::Update(float dt)
{
	coolDownTime -= dt;
	stateMachine->Update();
}