#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "..\CSC8503Common\GameWorld.h"
#include "..\CSC8503Common\PhysicsSystem.h"


namespace NCL {
	namespace CSC8503 {

		//typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class HumanObject :
			public GameObject
		{
		public:
			HumanObject(string n, Tag t = Tag::Null) : GameObject(n, t) { focusPlayer = nullptr; }

			void UpdateDistance();

			virtual void Update(float dt) override { if (stateMachine) stateMachine->Update(); }

			static void SetPlayerIterator(GameObjectIterator& first, GameObjectIterator& last) 
			{ 
				firstPlayer = first; 
				lastPlayer = last; 
			}
			static void SetPhysics(PhysicsSystem* p) { physics = p; }

			void SetStateMachine(StateMachine* s) { stateMachine = s; }
			StateMachine* GetStateMachine() const { return stateMachine; }

		protected:
			// TODO: GooseObjectIter
			static GameObjectIterator firstPlayer;
			static GameObjectIterator lastPlayer;
			StateMachine* stateMachine;
			float distance;
			GameObject* focusPlayer;
			static PhysicsSystem* physics;
		};
	}
}

