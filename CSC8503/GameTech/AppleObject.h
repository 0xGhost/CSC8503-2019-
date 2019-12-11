#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "GooseObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class AppleObject :
			public GameObject
		{
		public:
			AppleObject(string n = "", Tag t = Tag::Null, int score = 1) : GameObject(n, t), score(score) { }
			virtual void OnCollisionBegin(GameObject* otherObject) override;
			//void AddConstraintWithGoose(GooseObec )
			void RemoveConstraint();
			void SetOriginalPosition(Vector3 p) { originalPosition = p; }
			void SetWorld(GameWorld* w) { world = w; }
		protected:
			PositionConstraint* constraint;
			GooseObject* goose;
			GameWorld* world;
			int score;
			Vector3 originalPosition;
		};
	}
}

