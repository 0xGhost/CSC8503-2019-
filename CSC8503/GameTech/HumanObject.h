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
			HumanObject(string n, string t) : GameObject(n, t) { focusPlayer = nullptr; }

			void UpdateDistance();

			static void SetPlayerIterator(GameObjectIterator& first, GameObjectIterator& last) 
			{ 
				firstPlayer = first; 
				lastPlayer = last; 
			}

			static void SetPhysics(PhysicsSystem* p) { physics = p; }

		protected:
			static GameObjectIterator firstPlayer;
			static GameObjectIterator lastPlayer;
			float distance;
			GameObject* focusPlayer;
			static PhysicsSystem* physics;
		};
	}
}

