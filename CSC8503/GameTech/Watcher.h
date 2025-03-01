#pragma once
#include "Human.h"
namespace NCL {
	namespace CSC8503 {
		class WatcherObject :
			public HumanObject
		{
		public:
			WatcherObject(string name = "", string tag = "");

			virtual void Update(float dt) override;

			void SetAttackDistance(float d) { attackDistance = d; }
			void SetDetectedDistance(float d) { detectionDistance = d; }

		protected:
			StateMachine *stateMachine;
			float attackDistance;
			float detectionDistance;
			//GameObject *playerObject;
		};
	}
}