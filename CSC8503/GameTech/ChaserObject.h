#pragma once
#include "HumanObject.h"
namespace NCL {
	namespace CSC8503 {
		class ChaserObject :
			public HumanObject
		{
		public:
			ChaserObject(string name = "", string tag = "");

			//virtual void Update(float dt) override;

			void SetAttackDistance(float d) { searchDistance = d; }
			void SetDetectedDistance(float d) { detectionDistance = d; }
			virtual void InitOriginPosition();
			virtual void Movement(Vector3 direction, float force);

		protected:
			float searchDistance;
			float detectionDistance;
			Vector3 originPosition;
			float walkForce = 60;
			float runForce = 100;
			int state;
			//GameObject *playerObject;

		};
	}
}

