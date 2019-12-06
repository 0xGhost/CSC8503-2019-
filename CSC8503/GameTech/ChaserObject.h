#pragma once
#include "HumanObject.h"
#include "../CSC8503Common/NavigationPath.h"

namespace NCL {
	namespace CSC8503 {
		class ChaserObject;
		typedef std::function<void(Vector3 destination, ChaserObject*)> ChaserFunc;

		class ChaserObject :
			public HumanObject
		{
		public:
			ChaserObject(string name = "", string tag = "");

			//virtual void Update(float dt) override;

			void SetAttackDistance(float d) { searchDistance = d; }
			void SetDetectedDistance(float d) { detectionDistance = d; }
			void SetChaserFunc(ChaserFunc f) { getPath = f; }

			virtual void InitOriginPosition();
			virtual void Movement(Vector3 direction, float force);

			NavigationPath path;
		protected:
			float searchDistance;
			float detectionDistance;
			Vector3 originPosition;
			float walkForce = 80;
			float runForce = 200;
			int state;
			ChaserFunc getPath;
			//GameObject *playerObject;

		};
	}
}

