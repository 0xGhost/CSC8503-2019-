#pragma once
#include "..\CSC8503Common\GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class Human :
			public GameObject
		{
		public:
			Human(string n, string t) : GameObject(n, t) {}

			virtual void Update(float dt) override;

			static void SetPlayerIterator(GameObjectIterator& first, GameObjectIterator& last) { firstPlayer = first; lastPlayer = last; }
		protected:
			static GameObjectIterator& firstPlayer;
			static GameObjectIterator& lastPlayer;
			float distance;
			GameObject* focusPlayer;
		};
	}
}

