#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class AppleObject;
		class GooseObject :
			public GameObject
		{
		public:
			GooseObject(string n = "", Tag t = Tag::Null);

			virtual void Update(float dt) override;
			virtual void OnCollisionBegin(GameObject* otherObject) override;

			bool CheckApple() { return apple; }
			void PickApple(AppleObject* a) { apple = a; }
			void LostApple() { apple = nullptr; }
			void BackHomeWithApple(int s) { apple = nullptr; score += s; }
			int GetScore() const { return score; }
		protected:
			AppleObject* apple;
			int score;
			float waterFriciton;
			float groundFriction;
		};
	}
}