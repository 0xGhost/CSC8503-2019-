#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "StateMachine.h"
#include "State.h"

#include <vector>
#include <functional>

using std::vector;

namespace NCL {
	namespace CSC8503 {
		enum Tag {
			Null,
			PlayerTag,
			TileTag,
			WaterTag,
			HumanTag,
			BallTag,
			AppleTag,
			HomeTag,
			MaxTag,
		};

		class NetworkObject;
		class GameObject;
		typedef std::function<void(float dt, GameObject * g)> UpdateFunc;
		typedef std::function<void(GameObject *thisObj, GameObject* otherObj)> CollisionFunc;
		class GameObject {
		public:
			GameObject(string name = "", Tag tag = Tag::Null);
			~GameObject();

			float objTime;
			
			void SetBoundingVolume(CollisionVolume* vol) { boundingVolume = vol; }

			const CollisionVolume* GetBoundingVolume() const { return boundingVolume; }

			bool IsActive() const { return isActive; }
			bool IsStatic() const { return isStatic; }

			const string& GetName() const { return name; }
			const Tag GetTag() const { return tag; }

			const Transform& GetConstTransform() const { return transform; }
			Transform& GetTransform() { return transform; }

			RenderObject* GetRenderObject() const { return renderObject; }
			PhysicsObject* GetPhysicsObject() const { return physicsObject; }
			NetworkObject* GetNetworkObject() const { return networkObject; }

			void SetName(string n) { name = n; }
			void SetTag(Tag t) { tag = t; }

			void SetRenderObject(RenderObject* newObject) { renderObject = newObject; }
			void SetPhysicsObject(PhysicsObject* newObject) { physicsObject = newObject; }

			void SetActive(bool a) { isActive = a; }
			void SetStatic(bool s) { isStatic = s; }

			virtual void Update(float dt) { updateFunc(dt, this); }

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			unsigned int GetLayer() { return layer; }
			void SetLayer(unsigned int l) { layer = l; } // layer range: 0 ~ 31

			unsigned long long GetID() { return id; }
			static void ResetID() { nextId = 0; }

			void SetUpdateFunc(UpdateFunc func) { updateFunc = func; }

			bool GetBroadphaseAABB(Vector3& outsize) const;

			void UpdateBroadphaseAABB();

		protected:
			Transform			transform;

			CollisionVolume* boundingVolume;
			PhysicsObject* physicsObject;
			RenderObject* renderObject;
			NetworkObject* networkObject;


			bool	isActive;
			bool	isStatic;
			string	name;
			Tag tag;
			unsigned long long id;
			static unsigned long long nextId;
			unsigned int layer;
			UpdateFunc updateFunc;
			Vector3 broadphaseAABB;
		};
	}
}

