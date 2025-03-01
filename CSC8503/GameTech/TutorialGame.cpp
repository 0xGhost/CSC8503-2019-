#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "WatcherObject.h"
#include "ChaserObject.h"
#include "GooseObject.h"
#include "AppleObject.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/PushdownState.h"

#include <fstream>
#include <stdexcept>

#define check(a, b) ((a) & (b) ? 1 : 0)

using namespace NCL;
using namespace CSC8503;
using namespace std;

TutorialGame::TutorialGame() {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);
	physics->InitLayerCollisionMatrix(true);
	totalTime = 180;
	obstancleTime = 10;
	forceMagnitude = 100.0f;
	isPlaying = false;
	isDebuging = false;
	useGravity = false;
	inSelectionMode = false;
	mapSize = Vector2Int(10, 10);
	mapTiles = new int[100];
	memcpy(mapTiles, mapData, 100 * sizeof(int));
	Debug::SetRenderer(renderer);

	InitialiseAssets();
	InitMenuMachine();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete navMap;
	delete mapTiles;
	delete physics;
	delete renderer;
	delete world;

	delete mainMenu;
	delete inGameState;
	delete pauseState;

	if (server) delete server;
	if (client) delete client;
	if (packetReceiver) delete packetReceiver;
}

void NCL::CSC8503::TutorialGame::LoadMapData(const string& fileName) throw (invalid_argument)
{
	ifstream fileInput(Assets::DATADIR + fileName);

	if (fileInput.fail())
		throw invalid_argument("Fail to read the file \"" + fileName + "\".");

	fileInput >> mapSize.x >> mapSize.y;
	if (mapSize.x > 30 || mapSize.y > 30)
	{
		throw invalid_argument("File contain invalid data: map size too large");
	}
	delete[] mapTiles;
	mapTiles = new int[mapSize.x * mapSize.y];
	for (int x = 0; x < mapSize.x; x++)
	{
		for (int z = 0; z < mapSize.y; z++)
		{
			int type;
			fileInput >> type;
			if (type >= TileType::Invaild)
				throw invalid_argument("File contain invalid data: " + type);
			mapTiles[IndexOf(x, z)] = type;
		}
	}

	fileInput.close();
}

void NCL::CSC8503::TutorialGame::SaveMapData(const string& fileName) throw (invalid_argument)
{
	ofstream fileOutput(Assets::DATADIR + fileName);

	if (fileOutput.fail())
		throw invalid_argument("Fail to write the file \"" + fileName + "\".");
	fileOutput << mapSize.x << " " << mapSize.y << "\n";
	for (int x = 0; x < mapSize.x; x++)
	{
		for (int z = 0; z < mapSize.y; z++)
		{
			fileOutput << mapTiles[IndexOf(x, z)] << " ";
		}
		fileOutput << "\n";
	}
	fileOutput.close();
}

void NCL::CSC8503::TutorialGame::LoadMap()
{
	cout << "Enter the map name to load: ";
	//string fileName = "20map1";
	string fileName;
	//cin >> fileName;
	fileName = userInput;
	try
	{
		LoadMapData(fileName);
		InitWorld();
		selectionObject = nullptr;
		std::cout << "Map load successful." << endl;
	}
	catch (const invalid_argument& iae)
	{
		std::cout << "Unable to read data : " << iae.what() << "\n";
	}
}

void NCL::CSC8503::TutorialGame::SaveMap()
{
	cout << "Enter the map name to save: ";
	string fileName;
	cin >> fileName;
	try
	{
		SaveMapData(fileName);
		std::cout << "Map save successful." << endl;
	}
	catch (const invalid_argument& iae)
	{
		std::cout << "Unable to write data : " << iae.what() << "\n";
	}
}

char NCL::CSC8503::TutorialGame::GetKeyBoardInput()
{
	KeyboardKeys key = Window::GetKeyboard()->GetKeyPressed();
	if (key >= KeyboardKeys::A && key <= KeyboardKeys::Z)
		return (int)key - (int)KeyboardKeys::A + 'A';
	else if (key >= KeyboardKeys::NUM0 && key <= KeyboardKeys::NUM9)
		return (int)key - (int)KeyboardKeys::NUM0 + '0';
	if (key == KeyboardKeys::BACK)
		return 0;
	return -1;
}

int NCL::CSC8503::TutorialGame::GetKeyBoardNumber()
{
	KeyboardKeys key = Window::GetKeyboard()->GetKeyPressed();
	if (key >= KeyboardKeys::NUM0 && key <= KeyboardKeys::NUM9)
		return (int)key - (int)KeyboardKeys::NUM0;
	return -1;
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("CenteredGoose.msh", &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("RotatedCharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh", &appleMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	//InitWorld();
}

void NCL::CSC8503::TutorialGame::InitMenuMachine()
{
	mainMenu = new PushdownState([&](PushdownState** pushResult)
		{
			int i = 1;
			int offset = 40;
			Debug::Print("F1 to start a single-player game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("F2 to start a multi-player game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("F3 to enter edit and debug mode", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("ESC to exit the game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			/*
			char ch = GetKeyBoardInput();
			if(ch>0)
				userInput += ch;
			Debug::Print(userInput, Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			*/
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
			{


				//LoadMap();
				//InitWorld();
				//isPlaying = true;
				inputInstructions = "Enter the map name to load: ";
				*pushResult = userInputState;
				//pushResult = &selectMapMenu;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2))
			{
				//cout << "enter your name:" << endl;
				//cin >> playerName;
				*pushResult = multiplayerMenu;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3))
			{
				InitWorld();
				selectionObject = nullptr;
				//isDebuging = true;
				*pushResult = editModeState;
				return Push;
			}
			if (inputFinish)
			{
				inputFinish = false;
				LoadMap();
				InitWorld();
				//isPlaying = true;
				*pushResult = inGameState;
				//pushResult = &selectMapMenu;
				return Push;
			}
			return NoChange;
		});
	mainMenu->SetAwakeFunc([&]()
		{
			world->ClearAndErase();
			physics->Clear();
		});

	userInputState = new PushdownState([&](PushdownState** pushResult)
		{
			Debug::Print(inputInstructions, Vector2(10, renderer->GetWindowSize().y / 2 + 40), Vector4(0, 0, 0, 1));

			char ch = GetKeyBoardInput();
			if (ch > 0)
				userInput += ch;
			if (ch == 0)
				userInput.resize(userInput.size() - 1);
			Debug::Print(userInput, Vector2(10, renderer->GetWindowSize().y / 2), Vector4(0, 0, 0, 1));

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN))
			{
				inputFinish = true;
				return Pop;
			}
			return NoChange;
		});
	userInputState->SetAwakeFunc([&]
		{
			userInput.clear();
		});

	multiplayerMenu = new PushdownState([&](PushdownState** pushResult)
		{
			int i = 1;
			int offset = 40;
			Debug::Print("F1 to create a room", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("F2 to join a room", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("F3 to join a local server", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("B to back to main menu", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("ESC to exit the game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));

			NetworkBase::Initialise();

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
			{
				server = new GameServer(1234, 3);
				packetReceiver = new GamePacketReceiver("Server");
				server->RegisterPacketHandler(String_Message, packetReceiver);
				server->RegisterPacketHandler(FinalScore, packetReceiver);
				server->RegisterPacketHandler(Player_Connected, packetReceiver);
				server->RegisterPacketHandler(Player_Disconnected, packetReceiver);

				*pushResult = lobbyMenu;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2))
			{
				client = new GameClient();
				packetReceiver = new GamePacketReceiver("Client");
				client->RegisterPacketHandler(String_Message, packetReceiver);
				client->RegisterPacketHandler(Map, packetReceiver);
				client->RegisterPacketHandler(Time, packetReceiver);
				client->RegisterPacketHandler(Obstancle, packetReceiver);
				client->RegisterPacketHandler(Player_Connected, packetReceiver);
				client->RegisterPacketHandler(Player_Disconnected, packetReceiver);

				cout << "Enter server IP:" << endl;
				int a, b, c, d;
				cin >> a >> b >> c >> d;
				if (client->Connect(a, b, c, d, 1234))
				{
					*pushResult = lobbyMenu;
					return Push;
				}
				cout << "Connect failed" << endl;
				return NoChange;
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3))
			{
				client = new GameClient();
				packetReceiver = new GamePacketReceiver("Client");
				client->RegisterPacketHandler(String_Message, packetReceiver);
				client->RegisterPacketHandler(Map, packetReceiver);
				client->RegisterPacketHandler(Time, packetReceiver);
				client->RegisterPacketHandler(Obstancle, packetReceiver);
				client->RegisterPacketHandler(Player_Connected, packetReceiver);
				client->RegisterPacketHandler(Player_Disconnected, packetReceiver);

				if (client->Connect(127, 0, 0, 1, 1234))
				{
					*pushResult = lobbyMenu;
					return Push;
				}
				cout << "Connect failed" << endl;
				return NoChange;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
			{
				return PopUntilBottom;
			}
			return NoChange;
		});
	multiplayerMenu->SetAwakeFunc([&]() {
		if (server) {
			delete server;
			server = nullptr;
		}
		if (client) {
			delete client;
			client = nullptr;
		}
		if (packetReceiver) {
			delete packetReceiver;
			packetReceiver = nullptr;
		}
		});

	lobbyMenu = new PushdownState([&](PushdownState** pushResult)
		{
			if (server)
			{
				server->UpdateServer();
				Debug::Print("Server Lobby  Press F1 to start", Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));

				string s = "| PlayerS";
				//server->SendGlobalPacket(StringPacket("Server says hello !"));
				vector<int> clientsID = server->GetClientsID();
				for (int i = 0; i < clientsID.size(); i++)
				{
					s += " | Player" + std::to_string(clientsID[i]);
				}
				server->SendGlobalPacket(StringPacket(s));
				Debug::Print(s, Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));

				if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
				{
					inputInstructions = "Enter the map name to load: ";
					*pushResult = userInputState;
					/*LoadMap();
					server->SendGlobalPacket(MapPacket(mapTiles, mapSize.x, mapSize.y));
					server->UpdateServer();
					InitWorld();


					*pushResult = multiplayerInGameState;*/
					return Push;
				}
				if (inputFinish)
				{
					inputFinish = false;
					LoadMap();
					server->SendGlobalPacket(MapPacket(mapTiles, mapSize.x, mapSize.y));
					server->UpdateServer();
					InitWorld();
					//isPlaying = true;
					*pushResult = multiplayerInGameState;
					//pushResult = &selectMapMenu;
					return Push;
				}
			}
			else if (client)
			{
				client->UpdateClient();

				Debug::Print("Client Lobby  You are Player" + std::to_string(client->GetID()), Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
				string s = packetReceiver->GetNextString();
				if (!s.empty())
				{
					Debug::Print(s, Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
				}

				if (packetReceiver->MapReady())
				{
					packetReceiver->LoadMapData(mapSize.x, mapSize.y, mapTiles);
					InitWorld();
					*pushResult = multiplayerInGameState;
					return Push;
				}
			}


			return NoChange;
		}
	);

	multiplayerInGameState = new PushdownState([&](PushdownState** pushResult)
		{
			if (server)
			{
				Debug::Print("time:" + std::to_string((int)timeLeft),
					Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
				Debug::Print("score:" + std::to_string((*players.begin())->GetScore()),
					Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
				//Debug::Print("(P)ause", Vector2(renderer->GetWindowSize().x - 220, renderer->GetWindowSize().y - 20), Vector4(0.1f, 0.1f, 0.1f, 1));
				Debug::Print("(B)ack to menu", Vector2(renderer->GetWindowSize().x * 0.65f, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));

				timeLeft -= dt;
				obstancleCDTime += dt;
				GooseCameraMovement();
				GooseMovement();
				world->UpdateWorld(dt);
				if (obstancleCDTime > obstancleTime)
				{
					obstancleCDTime = 0;
					int index = rand() % lowGrounds.size();

					Vector3 position = Vector3(lowGrounds[index].x * 2 * TILESIZE, 15, lowGrounds[index].y * 2 * TILESIZE);
					position += worldOffset;
					AddObstancleToWorld(position, 2);
					server->SendGlobalPacket(ObstanclePacket(position, 2));
				}

				// send time
				server->SendGlobalPacket(TimePacket((int)timeLeft));
				server->UpdateServer();

				if (timeLeft < 0)
				{
					*pushResult = finishState;
					server->SendGlobalPacket(TimePacket(-1));
					server->UpdateServer();
					return Push;
				}
				if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
				{
					delete server;
					server = nullptr;
					return PopUntilBottom;
				}
			}
			else if (client)
			{
				client->UpdateClient();
				timeLeft = packetReceiver->GetTime();
				Debug::Print("time:" + std::to_string((int)timeLeft),
					Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
				Debug::Print("score:" + std::to_string((*players.begin())->GetScore()),
					Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
				//Debug::Print("(P)ause", Vector2(renderer->GetWindowSize().x - 220, renderer->GetWindowSize().y - 20), Vector4(0.1f, 0.1f, 0.1f, 1));
				Debug::Print("(B)ack to menu", Vector2(renderer->GetWindowSize().x - 420, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));

				GooseCameraMovement();
				GooseMovement();
				world->UpdateWorld(dt);

				if (packetReceiver->ObstancleReady())
				{
					Vector3 position;
					int score;
					packetReceiver->GetObstancleData(position, score);
					AddObstancleToWorld(position, score);
				}

				if (timeLeft < 0)
				{
					*pushResult = finishState;
					client->SendPacket(FinalScorePacket(client->GetID(), (*players.begin())->GetScore()));
					client->UpdateClient();
					return Push;
				}
				if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
				{
					delete client;
					client = nullptr;
					return PopUntilBottom;
				}
			}
			return NoChange;
		}
	);

	inGameState = new PushdownState([&](PushdownState** pushResult)
		{
			Debug::Print("time:" + std::to_string((int)timeLeft),
				Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("score:" + std::to_string((*players.begin())->GetScore()),
				Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("(P)ause", Vector2(renderer->GetWindowSize().x * 0.65f, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("(B)ack to menu", Vector2(renderer->GetWindowSize().x * 0.65f, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));

			timeLeft -= dt;
			obstancleCDTime += dt;
			GooseCameraMovement();
			GooseMovement();
			world->UpdateWorld(dt);
			if (obstancleCDTime > obstancleTime)
			{
				obstancleCDTime = 0;
				int index = rand() % lowGrounds.size();

				Vector3 position = Vector3(lowGrounds[index].x * 2 * TILESIZE, 15, lowGrounds[index].y * 2 * TILESIZE);
				position += worldOffset;
				AddObstancleToWorld(position, 2);
			}


			if (timeLeft < 0)
			{
				*pushResult = finishState;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P))
			{
				*pushResult = pauseState;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
			{
				return PopUntilBottom;
			}
			return NoChange;
		}
	);

	pauseState = new PushdownState([&](PushdownState** pushResult)
		{
			int i = 1;
			int offset = 40;
			Debug::Print("P to back to game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("F1 to reset game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("B to back to main menu", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("ESC to exit the game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P))
			{
				return Pop;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
			{
				InitWorld();
				return Pop;
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
			{
				return PopUntilBottom;
			}
			return NoChange;
		});

	editModeState = new PushdownState([&](PushdownState** pushResult)
		{
			if (!inSelectionMode) {
				world->GetMainCamera()->UpdateCamera(dt);
			}

			UpdateKeys();

			if (useGravity) {
				Debug::Print("(G)ravity on", Vector2(10, 40));
			}
			else {
				Debug::Print("(G)ravity off", Vector2(10, 40));
			}
			Debug::Print("cam pos:" + std::to_string((int)world->GetMainCamera()->GetPosition().x) +
				" " + std::to_string((int)world->GetMainCamera()->GetPosition().y) +
				" " + std::to_string((int)world->GetMainCamera()->GetPosition().z),
				Vector2(10, renderer->GetWindowSize().y - 120), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("cam pitch:" + std::to_string((int)world->GetMainCamera()->GetPitch()) +
				" yaw:" + std::to_string((int)world->GetMainCamera()->GetYaw()),
				Vector2(10, renderer->GetWindowSize().y - 160), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("(P)ause", Vector2(renderer->GetWindowSize().x * 0.65, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("(B)ack to menu", Vector2(renderer->GetWindowSize().x * 0.65, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
			Debug::Print("(L)ock goose", Vector2(renderer->GetWindowSize().x * 0.65, renderer->GetWindowSize().y - 120), Vector4(0.1f, 0.1f, 0.1f, 1));

			SelectObject();
			world->UpdateWorld(dt);

			if (isEditMode)
				EditSelectedObject();
			if (isPlaying)
			{
				GooseCameraMovement();
				GooseMovement();
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::L))
			{
				isPlaying = !isPlaying;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P))
			{
				*pushResult = pauseState;
				return Push;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
			{
				return Pop;
			}
			return NoChange;
		});

	editModeState->SetAwakeFunc([&]
		{
			inSelectionMode = false;
		});

	finishState = new PushdownState([&](PushdownState** pushResult)
		{
			Debug::Print("Game Finish", Vector2(renderer->GetWindowSize().x / 2 - 220, renderer->GetWindowSize().y / 2), Vector4(0, 0, 0, 1));
			Debug::Print("Your Score:" + std::to_string((int)goose->GetScore()), Vector2(renderer->GetWindowSize().x / 2 - 220, renderer->GetWindowSize().y / 2 - 40), Vector4(0, 0, 0, 1));
			int offset = 40;
			int i = 1;
			if (server)
			{
				string finalStr = "PlayerS:" + std::to_string(players[0]->GetScore()) + packetReceiver->GetFinalScoreMessage();
				server->SendGlobalPacket(StringPacket(finalStr));
				server->UpdateServer();
				Debug::Print(finalStr, Vector2(renderer->GetWindowSize().x / 2 - 620, renderer->GetWindowSize().y / 2 - 80), Vector4(0, 0, 0, 1));
			}
			else if (client)
			{
				client->UpdateClient();
				string s = packetReceiver->GetNextString();
				if (!s.empty())
					finalResult = s;
				Debug::Print(finalResult, Vector2(renderer->GetWindowSize().x / 2 - 620, renderer->GetWindowSize().y / 2 - 80), Vector4(0, 0, 0, 1));

			}
			else
			{
				Debug::Print("F1 to play Again", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
				if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
				{
					InitWorld();
					return Pop;
				}
			}
			Debug::Print("B to back to main menu", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));
			Debug::Print("ESC to exit the game", Vector2(10, renderer->GetWindowSize().y - (i++) * offset), Vector4(0, 0, 0, 1));



			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B))
			{
				return PopUntilBottom;
			}
			return NoChange;
		});



	gameStateManager.InitState(mainMenu);
}

void TutorialGame::UpdateGame(float dt) {

	this->dt = dt;
	gameStateManager.Update();


	/*if (isDebuging)
	{
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		if (lockedObject != nullptr) {
			LockedCameraMovement();
		}

		UpdateKeys();

		if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(10, 40));
		}
		else {
			Debug::Print("(G)ravity off", Vector2(10, 40));
		}
		Debug::Print("cam pos:" + std::to_string((int)world->GetMainCamera()->GetPosition().x) +
			" " + std::to_string((int)world->GetMainCamera()->GetPosition().y) +
			" " + std::to_string((int)world->GetMainCamera()->GetPosition().z),
			Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));
		Debug::Print("cam pitch:" + std::to_string((int)world->GetMainCamera()->GetPitch()) +
			" yaw:" + std::to_string((int)world->GetMainCamera()->GetYaw()),
			Vector2(10, renderer->GetWindowSize().y - 120), Vector4(0.1f, 0.1f, 0.1f, 1));
		Debug::Print("score:" + std::to_string((*players.begin())->GetScore()),
			Vector2(800, renderer->GetWindowSize().y - 60), Vector4(0.1f, 0.1f, 0.1f, 1));

		SelectObject();
		if (isEditMode)
			EditSelectedObject();
		else
			MoveSelectedObject();
	}*/
	//world->UpdateWorld(dt);

	renderer->Update(dt);
	physics->Update(dt);


	Debug::FlushRenderables();
	renderer->Render();

}

void TutorialGame::UpdateKeys() {
	int offset = 40;
	int i = 4;
	Debug::Print("F1 to reset world. F2 to reset camera.", Vector2(10, i++ * offset));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	Debug::Print("F3 to save the map. F4 to load a map.", Vector2(10, i++ * offset));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) { // Save Map
		cout << "Enter the map name to save: ";
		string fileName;
		cin >> fileName;
		try
		{
			SaveMapData(fileName);
			std::cout << "Map save successful." << endl;
		}
		catch (const invalid_argument& iae)
		{
			std::cout << "Unable to write data : " << iae.what() << "\n";
		}
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F4)) { // Load Map
		cout << "Enter the map name to load: ";
		string fileName;
		cin >> fileName;
		try
		{
			LoadMapData(fileName);
			InitWorld();
			selectionObject = nullptr;
			std::cout << "Map load successful. Press F1 to use new map" << endl;
		}
		catch (const invalid_argument& iae)
		{
			std::cout << "Unable to read data : " << iae.what() << "\n";
		}
	}

	Debug::Print("F5 to toggle edit mode.", Vector2(10, i++ * offset));

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F5)) { // edit mode
		isEditMode = isEditMode ? false : true;
	}

	Debug::Print("F6 to create a new map.", Vector2(10, i++ * offset));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F6)) // create new map
	{
		cout << "Enter map size(two integer): ";
		int x, y;
		cin >> x >> y;
		if (x > 30 || y > 30)
		{
			cout << "Map size cannot larger than 30" << endl;
			return;
		}
		mapSize = Vector2Int(x, y);
		delete[] mapTiles;
		mapTiles = new int[mapSize.x * mapSize.y];
		memset(mapTiles, TileType::LowGround, mapSize.x * mapSize.y * sizeof(int));
		InitWorld();
		selectionObject = nullptr;
		isEditMode = true;
	}

	Debug::Print("F7 to toggle dynamic quad tree.", Vector2(10, i++ * offset));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) physics->quadTreeDebug = !physics->quadTreeDebug;
	Debug::Print("F8 to toggle static quad tree.", Vector2(10, i++ * offset));
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) physics->staticTreeDebug = !physics->staticTreeDebug;

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	/*if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}*/
}

void TutorialGame::LockedObjectMovement() {
	if (!selectionObject) return;
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	float f = 100;
	Vector3 front = -selectionObject->GetTransform().GetForward() * 0.1;
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		//selectionObject->GetPhysicsObject()->AddForce(-rightAxis * f);
		selectionObject->GetPhysicsObject()->AddForceAtLocalPosition(-rightAxis * f, front);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		//selectionObject->GetPhysicsObject()->AddForce(rightAxis * f);
		selectionObject->GetPhysicsObject()->AddForceAtLocalPosition(rightAxis * f, front);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		//selectionObject->GetPhysicsObject()->AddForce(fwdAxis * f);
		selectionObject->GetPhysicsObject()->AddForceAtLocalPosition(fwdAxis * f, front);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		//selectionObject->GetPhysicsObject()->AddForce(-fwdAxis * f);
		selectionObject->GetPhysicsObject()->AddForceAtLocalPosition(-fwdAxis * f, front);
	}
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
	}
}

void NCL::CSC8503::TutorialGame::GooseCameraMovement()
{
	Vector3 objPos = goose->GetTransform().GetWorldPosition();
	Vector3 camPos = objPos + lockedOffset;

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera()->SetPosition(camPos);
	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);
}

void NCL::CSC8503::TutorialGame::GooseMovement()
{
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	float f = 100;
	Vector3 front = -goose->GetTransform().GetForward() * 0.1;
	Vector3 finalDir;
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		finalDir += -rightAxis;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		finalDir += rightAxis;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		finalDir += fwdAxis;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		finalDir += -fwdAxis;
	}
	goose->GetPhysicsObject()->AddForceAtLocalPosition(finalDir.Normalised() * f, front);

}


void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {

	if (selectionObject) {
		Debug::Print("ID:" + std::to_string(selectionObject->GetID()) + " name:" + selectionObject->GetName(),
			Vector2(10, renderer->GetWindowSize().y - 40), Vector4(0.1f, 0.1f, 0.1f, 1));

		Debug::Print("pos:" + std::to_string((int)selectionObject->GetTransform().GetWorldPosition().x) +
			" " + std::to_string((int)selectionObject->GetTransform().GetWorldPosition().y) +
			" " + std::to_string((int)selectionObject->GetTransform().GetWorldPosition().z) +
			" rot:" + std::to_string((int)selectionObject->GetTransform().GetWorldOrientation().ToEuler().x) +
			" " + std::to_string((int)selectionObject->GetTransform().GetWorldOrientation().ToEuler().y) +
			" " + std::to_string((int)selectionObject->GetTransform().GetWorldOrientation().ToEuler().z)
			, Vector2(10, renderer->GetWindowSize().y - 80), Vector4(0.1f, 0.1f, 0.1f, 1));

	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(selectionColor);
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
			Debug::DrawLine(ray.GetPosition(), (ray.GetDirection() * 5000 + ray.GetPosition()));
			RayCollision closestCollision;
			if (physics->Raycast(ray, closestCollision, true, ~(1 << 6))) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionColor = selectionObject->GetRenderObject()->GetColour();
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 0.2f, 0.2f, 1));



#if 0	// Futher Work 1-2
				Debug::DrawLine(selectionObject->GetTransform().GetWorldPosition(), (closestCollision.collidedAt - selectionObject->GetTransform().GetWorldPosition()) * 500 + selectionObject->GetTransform().GetWorldPosition());
				//Debug::DrawLine(selectionObject->GetTransform().GetWorldPosition(), (closestCollision.collidedAt -selectionObject->GetTransform().GetWorldPosition()), 500);
				int layer = selectionObject->GetLayer();
				selectionObject->SetLayer(0);
				Vector3 a = selectionObject->GetTransform().GetWorldPosition();
				Vector3 b = selectionObject->GetTransform().GetForward();
				ray = Ray(selectionObject->GetTransform().GetWorldPosition(), selectionObject->GetTransform().GetForward());
				Debug::DrawLine(ray.GetPosition(), (ray.GetDirection() * 5000 + ray.GetPosition()));
				RayCollision closestCollision1;
				if (world->Raycast(ray, closestCollision1, true, ~0U << 1))
				{
					GameObject* seeingObject = (GameObject*)closestCollision1.node;
					seeingObject->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
			}
				selectionObject->SetLayer(layer);
#endif
				return true;
		}
			else {
				return false;
			}
	}
		/*if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}*/
}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
		}

void NCL::CSC8503::TutorialGame::EditSelectedObject()
{
	if (!selectionObject) {
		return;// we haven �t selected anything !
	}
	int offset = 40;
	int i = 6;
	float xOffset = 0.75f;
	Debug::Print("N:Low", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("H:High", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("I:Watcher", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("K:Keeper", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("J:Goose", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("U:Water", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	Debug::Print("O:Apple", Vector2(renderer->GetWindowSize().x * xOffset, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.5, 0.5, 0.5, 0.9));
	//Debug::Print("N:Low H:High I:Watcher K:Keeper ", Vector2(renderer->GetWindowSize().x - 300, renderer->GetWindowSize().y - (i++) * offset), Vector4(0.2, 0.2, 0.2, 0.9));
	//Debug::Print("J:Goose U:Water O:Apple", Vector2(renderer->GetWindowSize().x - 300, 60), Vector4(0.2, 0.2, 0.2, 0.9));
	Vector3 pos = selectionObject->GetTransform().GetWorldPosition();
	pos -= worldOffset;
	int x = pos.x / (TILESIZE * 2);
	int y = pos.z / (TILESIZE * 2);
	TileType type = (TileType)mapTiles[IndexOf(x, y)];
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::N)))
	{
		type = TileType::LowGround;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::H)))
	{
		type = TileType::HighGround;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::I)))
	{
		type = TileType::Watcher;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::K)))
	{
		type = TileType::Chaser;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::J)))
	{
		type = TileType::Goose;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::U)))
	{
		type = TileType::Water;
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::O)))
	{
		type = TileType::Apple;
	}
	if (type == mapTiles[IndexOf(x, y)]) return;
	mapTiles[IndexOf(x, y)] = type;
	InitWorld();
	selectionObject = nullptr;

}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude),
		Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 10.0f;

	if (!selectionObject) {
		return;// we haven �t selected anything !
	}
	// Push the selected object !
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(
			*world->GetMainCamera());

		RayCollision closestCollision;
		if (physics->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
#if 0 // futher work, can be used for debug
	if (!inSelectionMode) return;
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::W)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * forceMagnitude * dt);
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::S)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * forceMagnitude * dt);
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::A)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * forceMagnitude * dt);
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::D)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * forceMagnitude * dt);
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * forceMagnitude * dt);
	}
	if ((Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)))
	{
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * forceMagnitude * dt);
	}
#endif
			}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {

	cout << "init" << endl;
	world->ClearAndErase();
	physics->Clear();

	//return;


#if 1 // coursework
	GameObject::ResetID();
	physics->SetLayerCollision(2, 2, false); // cube - cube
	physics->SetLayerCollision(5, 5, false); // freeBall - freeBall
	physics->SetLayerCollision(5, 2, false); // freeBall - cube
	physics->SetLayerCollision(5, 3, false); // freeBall - enemy
	physics->SetLayerCollision(4, 3, false); // ball - enemy

	physics->SetWorldSize(Vector3(mapSize.x * TILESIZE, 20, mapSize.y * TILESIZE));
	lightPos = Vector3(-200.0f, 160.0f, -200.0f);
	renderer->SetLightPosition(lightPos);


#pragma region ShadowProjMat

	Vector2 lightv2 = Vector2(-lightPos.x, -lightPos.z);

	float d = sqrt(mapSize.x * mapSize.x * 100 + mapSize.y * mapSize.y * 100) / 2;
	float topdownD = abs(lightPos.y / lightPos.Length() * (d + abs(lightv2.Length() / lightPos.y * (TILESIZE + HIGHGROUNDY) * 2)));
	float nearfarD = lightPos.y / lightPos.Length() * d;

	renderer->SetShadowProjMatrix(Matrix4::Orthographic(lightPos.Length() - nearfarD * 2, lightPos.Length() + nearfarD * 2, d + 1, -d - 1, topdownD, -topdownD));
	//renderer->SetShadowProjMatrix(Matrix4::Perspective(20, 500, 1, 70));
#pragma endregion
	worldOffset = Vector3(-mapSize.x * TILESIZE + TILESIZE, 0, -mapSize.y * TILESIZE + TILESIZE);
	//int* mapTemp = mapTiles;
	players.clear();
	lowGrounds.clear();
	freeBalls.clear();
	for (int x = 0; x < mapSize.x; x++)
	{
		for (int z = 0; z < mapSize.y; z++)
		{
			//mapTemp[IndexOf(x, z)] &= (TileType::LowGround | TileType::HighGround);
			// TODO: cube expand
			AddTileToWorld(x, z);
		}
	}
	navMap = new NavigationGrid(TILESIZE * 2, mapSize.x, mapSize.y, mapTiles);

	HumanObject::SetPhysics(physics);
	HumanObject::SetPlayerIterator(players.begin(), players.end());
	if (players.size() > 0)
		goose = players[0];
	physics->InitQuadTree();
	useGravity = true;
	physics->UseGravity(true);
	obstancleCDTime = obstancleTime;
	timeLeft = totalTime;
	world->UpdateWorld(dt);
#else
	BridgeConstraintTest();
	//InitMixedGridWorld(10, 10, 6.0f, 6.0f);
	GameObject* goose = AddGooseToWorld(Vector3(30, 2, 0));
	//goose->SetLayer(2);

	GameObject* apple = AddAppleToWorld(Vector3(35, 2, 0));
	//apple->SetLayer(3);

	AddParkKeeperToWorld(Vector3(40, 5, 0));
	AddCharacterToWorld(Vector3(45, 5, 0));

	AddFloorToWorld(Vector3(0, -2, 0));
#endif
}

//From here on it's functions to add in objects to the world!

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool isHollow) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia(isHollow);

	world->AddGameObject(sphere);

	return sphere;
}

void NCL::CSC8503::TutorialGame::AddTileToWorld(int x, int z)
{
	float y = check(mapTiles[IndexOf(x, z)], TileType::Water) * WATERY
		+ check(mapTiles[IndexOf(x, z)], TileType::HighGround) * HIGHGROUNDY;
	Vector4 cubeColor = Vector4(0.2f, 0.2f, 0.9f, 0.6f) * check(mapTiles[IndexOf(x, z)], TileType::Water)
		+ Vector4(0.3f, 1, 0.3f, 1) * check(mapTiles[IndexOf(x, z)], TileType::LowGround)
		+ Vector4(0.8f, 0.5f, 0.3f, 1) * check(mapTiles[IndexOf(x, z)], TileType::HighGround);
	Vector3 position = Vector3(x * 2 * TILESIZE, y, z * 2 * TILESIZE);
	position += worldOffset;
	Vector3 cubeDims = Vector3(TILESIZE, TILESIZE + y, TILESIZE);
	GameObject* onTile;
	if (mapTiles[IndexOf(x, z)] == TileType::LowGround)
	{
		lowGrounds.push_back(Vector2(x, z));
	}
	if (mapTiles[IndexOf(x, z)] == TileType::Apple)
	{
		onTile = AddAppleToWorld(position + Vector3(0, 8 + y, 0));
	}
	if (mapTiles[IndexOf(x, z)] == TileType::Goose)
	{
		onTile = AddGooseToWorld(position + Vector3(0, 8 + y, 0));
		AddBoxTriggerToWorld(position + Vector3(0, 10 + y, 0), Vector3(TILESIZE, TILESIZE, TILESIZE), Vector4(0.8f, 0.2f, 0.2f, 1));
	}
	if (mapTiles[IndexOf(x, z)] == TileType::Chaser)
	{
		onTile = AddChaserToWorld(position + Vector3(0, 10 + y, 0));
	}
	if (mapTiles[IndexOf(x, z)] == TileType::Watcher)
	{
		onTile = AddWatcherToWorld(position + Vector3(0, 10 + y, 0));
	}
	GameObject* cube = AddCubeToWorld(position, cubeDims, 0, cubeColor);
	cube->SetLayer(2);
	cube->SetTag(check(mapTiles[IndexOf(x, z)], TileType::Water) ? Tag::WaterTag : Tag::TileTag);
	cube->SetStatic(true);
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, Vector4 color) {
	GameObject* cube = new GameObject("cube");

	//OBBVolume* volume = new OBBVolume(dimensions);
	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr/*basicTex*/, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	cube->GetRenderObject()->SetColour(color);

	world->AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::TutorialGame::AddObstancleToWorld(const Vector3& position, int score)
{
	AppleObject* apple = new AppleObject("Apple", Tag::AppleTag, 2);

	SphereVolume* volume = new SphereVolume(1.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(6, 6, 6));
	apple->GetTransform().SetWorldPosition(position);
	apple->SetOriginalPosition(position);
	apple->SetWorld(world);
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->GetRenderObject()->SetColour(Vector4(0.7f, 0.1f, 0.1f, 1));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));
	apple->SetLayer(7);
	apple->GetPhysicsObject()->SetInverseMass(3.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	float size = 1.0f;
	float inverseMass = 1.0f;

	GooseObject* goose = new GooseObject("Goose", Tag::PlayerTag);

	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size, size, size));
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));
	goose->GetPhysicsObject()->SetElasticity(0.0f);
	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goose);

	players.push_back(goose);

	return goose;
}

GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position)
{
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	GameObject* keeper = new GameObject("Keeper");

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(keeper);

	return keeper;
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position, const int r) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	GameObject* character = r > 0.5f ? (GameObject*)(new ChaserObject("Chaser")) : (GameObject*)(new WatcherObject("Watcher"));//new GameObject(r > 0.5f ? "Chaser" : "Watcher");
	character->SetTag(HumanTag);
	SphereVolume* volume = new SphereVolume(0.9f * meshSize);//new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* NCL::CSC8503::TutorialGame::AddWatcherToWorld(const Vector3& position)
{
	WatcherObject* watcher = (WatcherObject*)AddCharacterToWorld(position, 0);
	watcher->SetWatcherFunc([&](Vector3 direction, GameObject* g) {
		AddBallToWorld(g->GetTransform().GetWorldPosition() + direction * 4.0f + Vector3(0, 5, 0), direction);
		});
	watcher->SetLayer(3);
	return watcher;
}

GameObject* NCL::CSC8503::TutorialGame::AddChaserToWorld(const Vector3& position)
{
	ChaserObject* chaser = (ChaserObject*)AddCharacterToWorld(position, 1);
	chaser->SetLayer(3);
	chaser->InitOriginPosition();
	chaser->SetChaserFunc([&](Vector3 destination, ChaserObject* g) {
		Vector3 pos = g->GetTransform().GetWorldPosition();
		pos -= worldOffset;
		int x = (pos.x / (TILESIZE * 2) + 0.5);
		int y = (pos.z / (TILESIZE * 2) + 0.5);
		destination -= worldOffset;
		int dx = (destination.x / (TILESIZE * 2) + 0.5);
		int dy = (destination.z / (TILESIZE * 2) + 0.5);
		navMap->FindPath(y, x, dy, dx, g->path);
		g->worldOffset = worldOffset;
		});
	return chaser;
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	AppleObject* apple = new AppleObject("Apple", Tag::AppleTag);

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);
	apple->SetOriginalPosition(position);
	apple->SetWorld(world);
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->GetRenderObject()->SetColour(Vector4(1.0f, 0.3f, 0.3f, 1));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));
	apple->SetLayer(7);
	apple->GetPhysicsObject()->SetInverseMass(5.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* NCL::CSC8503::TutorialGame::AddBallToWorld(const Vector3& position, const Vector3& direction)
{
	GameObject* ball;
	if (freeBalls.empty())
	{
		ball = AddSphereToWorld(position, 1.0f, 1.0f / 2.0f);
		ball->SetTag(BallTag);
		ball->GetPhysicsObject()->SetFriction(0.2f);
		ball->GetPhysicsObject()->SetElasticity(0.8f);
		ball->SetUpdateFunc([&](float dt, GameObject* g) {
			g->objTime += dt;
			if (g->objTime > 10)
				RemoveBall(g);

			});
	}
	else
	{
		ball = freeBalls.back();
		freeBalls.pop_back();
		ball->GetTransform().SetWorldPosition(position);
		ball->GetPhysicsObject()->ClearForces();
		ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		ball->objTime = 0;
	}
	ball->SetLayer(4);
	ball->GetPhysicsObject()->ApplyLinearImpulse(direction * 100);//AddForce(direction * 3000);// ->AddForce(-direction * 5000);
	return ball;
}

GameObject* NCL::CSC8503::TutorialGame::AddBoxTriggerToWorld(const Vector3& position, Vector3 dimensions, Vector4 color)
{
	GameObject* box = new GameObject("BoxTrigger", HomeTag);

	AABBVolume* volume = new AABBVolume(dimensions);

	box->SetBoundingVolume((CollisionVolume*)volume);
	box->SetLayer(6); // raycast ignore

	box->GetTransform().SetWorldPosition(position);
	box->GetTransform().SetWorldScale(dimensions);

	//box->SetRenderObject(new RenderObject(&box->GetTransform(), nullptr, nullptr/*basicTex*/, basicShader));
	box->SetPhysicsObject(new PhysicsObject(&box->GetTransform(), box->GetBoundingVolume()));
	box->GetPhysicsObject()->SetTrigger(true);
	box->GetPhysicsObject()->SetInverseMass(0);
	//box->GetRenderObject()->SetColour(color);

	box->SetUpdateFunc([&](float dt, GameObject* g) {
		Debug::DrawCube(g->GetTransform().GetWorldPosition(), g->GetTransform().GetLocalScale(), Vector4(1.0f, 0.3f, 0.3f, 1));
		});
	world->AddGameObject(box);

	return box;
}

void NCL::CSC8503::TutorialGame::RemoveBall(GameObject* o)
{
	freeBalls.push_back(o);

	//o->GetTransform().SetWorldPosition(Vector3(0,0,0));
	o->SetLayer(5);
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(3, 3, 3);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float	invCubeMass = 5;
	int		numLinks = 25;
	float	maxDistance = 30;
	float	cubeDistance = 20;

	Vector3 startPos = Vector3(500, 1000, 500);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions = Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, 10.0f);
	GameObject* newFloor = AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

