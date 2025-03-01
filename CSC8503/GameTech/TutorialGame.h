#pragma once
#include "GameTechRenderer.h"
#include "../../Common/Assets.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "GooseObject.h"
#include "..//CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"
#include "GamePacketReceiver.h"

const int TILESIZE = 5;
const float WATERY = -0.2f;
const float HIGHGROUNDY = 4;// -WATERY;



namespace NCL {
	namespace CSC8503 {
		struct Vector2Int
		{
			int x;
			int y;
			Vector2Int(int x, int y)
			{
				this->x = x;
				this->y = y;
			}
			Vector2Int()
			{
				this->x = 0;
				this->y = 0;
			}
		};
		enum TileType
		{
			
			LowGround = 1, // 1 << 0

			//Chaser = 2, // 1 << 1 
			Chaser = 3, // 1 << 0 & 1 << 1 

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
			void LoadMap();
			void SaveMap();
			int IndexOf(int x, int y) { return x * mapSize.y + y; }
			char GetKeyBoardInput();
			int GetKeyBoardNumber();

			void InitialiseAssets();
			void InitMenuMachine();

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
			void EditSelectedObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();

			void GooseCameraMovement();
			void GooseMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool isHollow = false);
			void AddTileToWorld(int x, int z);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, Vector4 color = Vector4(1,1,1,1));
			//IT'S HAPPENING
			GameObject* AddObstancleToWorld(const Vector3& position, int score);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position, const int r = 0);
			GameObject* AddWatcherToWorld(const Vector3& position);
			GameObject* AddChaserToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			GameObject* AddBallToWorld(const Vector3& position, const Vector3& direction);
			GameObject* AddBoxTriggerToWorld(const Vector3& position, Vector3 dimensions, Vector4 color = Vector4(1, 1, 1, 1));
			void RemoveBall(GameObject* o);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool isEditMode;
			bool isPlaying;
			bool isDebuging;

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
			float totalTime;
			float timeLeft;
			float obstancleTime;
			float obstancleCDTime;
			Vector2Int mapSize;
			int* mapTiles;
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
			vector<Vector2> lowGrounds;
			Vector3 lightPos;

			GooseObject* goose;
			vector<GooseObject*> players;
			vector<GameObject*> freeBalls;
			NavigationMap* navMap;
			
			PushdownMachine gameStateManager;
			PushdownState* mainMenu;
			PushdownState* lobbyMenu;
			PushdownState* multiplayerMenu;
			PushdownState* multiplayerInGameState;
			PushdownState* selectMapMenu;
			PushdownState* editModeState;
			PushdownState* inGameState;
			PushdownState* pauseState;
			PushdownState* finishState;
			PushdownState* userInputState;

			GameServer* server;
			GameClient* client;
			GamePacketReceiver* packetReceiver;
			GamePacketReceiver* serverPacketReceiver;
			string playerName;
			string finalResult;
			bool inputFinish;
			string userInput;
			string inputInstructions;
			vector<string> clientNames;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset = Vector3(0, 100, -20);//Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			Vector3 worldOffset;
		};
	}
}

