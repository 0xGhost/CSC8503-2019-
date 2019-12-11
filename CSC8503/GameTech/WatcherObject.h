#pragma once
#include "HumanObject.h"
namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(Vector3, GameObject*)> WatcherFunc;
		class WatcherObject :
			public HumanObject
		{
		public:
			WatcherObject(string name = "", Tag t = Tag::Null);

			virtual void Update(float dt) override;

			void SetAttackDistance(float d) { attackDistance = d; }
			void SetDetectedDistance(float d) { detectionDistance = d; }
			void SetWatcherFunc(WatcherFunc f) { throwBall = f; }
		protected:
			//StateMachine *stateMachine;
			float attackDistance;
			float detectionDistance;
			const float totalCoolDownTime = 1.0f;
			float coolDownTime;
			int ready;
			WatcherFunc throwBall;
			
			//GameObject *playerObject;
		};
	}
}