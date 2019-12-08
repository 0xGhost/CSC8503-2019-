#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class AppleObject;
		class GooseObject :
			public GameObject
		{
		public:
			GooseObject(string n = "", Tag t = Tag::Null) : GameObject(n, t) { apple = nullptr; }

			virtual void Update(float dt) override;
			virtual void OnCollisionBegin(GameObject* otherObject) override;

			bool CheckApple() { return apple; }
			void GetApple(AppleObject* a) { apple = a; }
			void LostApple() { apple = nullptr; }

		protected:
			AppleObject* apple;
		};
	}
}