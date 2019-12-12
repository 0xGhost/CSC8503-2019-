#pragma once
#include "..\CSC8503Common\PositionConstraint.h"
namespace NCL {
	namespace CSC8503 {
		class FrontPositionConstraint :
			public PositionConstraint
		{
		public:
			FrontPositionConstraint(GameObject* a, GameObject* b, float d) : PositionConstraint(a, b, d) {}
			void UpdateConstraint(float dt) override;

		};
	}
}

