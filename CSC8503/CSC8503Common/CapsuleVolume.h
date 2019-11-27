#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class CapsuleVolume :
		public CollisionVolume
	{
	public:
		CapsuleVolume(float capslueHeight = 1.0f, float capsuleRadius = 1.0f) {
			type = VolumeType::Capsule;
			radius = capsuleRadius;
			height = capslueHeight;
		}
		~CapsuleVolume() {}

		float GetRadius() const {
			return radius;
		}
		float GetHeight() const {
			return height;
		}
	protected:
		float	radius;
		float	height;
	};
}

