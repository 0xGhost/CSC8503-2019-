#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
const int TILESIZE = 5;
const float WATERY = -0.5f;
const float HIGHGROUNDY = 4;// -WATERY;




namespace NCL {
	namespace CSC8503 {
		enum TileType
		{
			
			LowGround = 1, // 1 << 0

			//Keeper = 2, // 1 << 1 
			Keeper = 3, // 1 << 0 & 1 << 1 

			HighGround = 4, // 1 << 2

			//Watcher = 8, // 1 << 2
			Watcher = 12, // 1 << 2 & 1 << 3

			//Apple = 16, // 1 << 4
			Apple = 17, // 1 << 0 & 1 << 4

			//Goose = 32, // 1 << 5
			Goose = 33, // 1 << 0 & 1 << 5

			Water = 64, // 1 << 6

			Invaild
		};

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void LoadMapData(const string& fileName);
			void SaveMapData(const string& fileName);

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest();
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool isHollow = false);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, Vector4 color = Vector4(0,0,0,1));
			//IT'S HAPPENING
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);


			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;
			Vector4 selectionColor;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			float dt;
			Vector2 mapSize;
			vector<int> mapTiles;
			int mapData[100] = 
			{
				4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
				4, 1, 1, 1, 1, 4, 1, 4, 4, 4,
				4, 1, 1, 1, 1, 4, 1, 12, 4, 4,
				4, 1, 4, 1, 1, 4, 1, 1, 1, 17,
				4, 1, 4, 64, 64, 1, 1, 1, 3, 4,
				33, 1, 4, 64, 64, 4, 4, 4, 1, 4,
				4, 1, 4, 64, 64, 4, 4, 4, 1, 4,
				4, 1, 4, 64, 64, 1, 1, 1, 1, 4,
				4, 1, 4, 1, 1, 1, 1, 1, 1, 4, 
				4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

			};

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
		};
	}
}

