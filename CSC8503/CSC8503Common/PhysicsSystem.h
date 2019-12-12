#pragma once
#include "../CSC8503Common/GameWorld.h"
#include <set>

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem {
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) { applyGravity = state; }

			void SetGlobalDamping(float d) { globalDamping = d; }

			void SetGravity(const Vector3& g) { gravity = g; }

			void SetWorldSize(const Vector3& ws) { worldSize = ws; }

			void InitQuadTree();

			void InitLayerCollisionMatrix(bool b) { memset(LayerCollisionMatrix, b, sizeof(bool) * 32 * 32); }
			// layer range: 0 ~ 31
			bool CheckLayerCollision(unsigned int a, unsigned int b) const { return LayerCollisionMatrix[a][b]; }
			void SetLayerCollision(unsigned int a, unsigned int b, bool c) { LayerCollisionMatrix[a][b] = c; LayerCollisionMatrix[b][a] = c; }

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false, unsigned int layerMask = ~0) const;
			
			bool quadTreeDebug = false;
			bool staticTreeDebug = false;
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt, bool useFriction = true);

			void UpdateConstraints(float dt);

			
			void UpdateQuadTree();
			void UpdateCollisionList();
			void UpdateObjectAABBs();

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;
			void ResolveSpringCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;

			GameWorld& gameWorld;
			QuadTree<GameObject*>* quadTree;
			QuadTree<GameObject*>* staticQuadTree;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;
			float	frameDT;
			Vector3 worldSize;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo>		broadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo>	broadphaseCollisionsVec;
			bool useBroadPhase = true;

			int numCollisionFrames = 5;

			bool LayerCollisionMatrix[32][32];
		};
	}
}

