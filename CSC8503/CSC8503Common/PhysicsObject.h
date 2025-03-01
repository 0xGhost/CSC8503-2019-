	#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Matrix3.h"

using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			float GetFriction() const {
				return friction;
			}

			float GetElasticity() const {
				return elasticity;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtLocalPosition(const Vector3& addedForce, const Vector3& position);
			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);


			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void SetElasticity(const float& e) {
				elasticity = e;
			}

			void SetFriction(const float& f) {
				friction = f;
			}

			void InitCubeInertia();
			void InitSphereInertia(bool isHollow = false);

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

			bool IsTrigger() { return isTrigger; }
			void SetTrigger(bool t) { isTrigger = t; }

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			float inverseMass;
			float elasticity;
			float friction;
			bool isTrigger;

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			

			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}

