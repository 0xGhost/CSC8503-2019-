#pragma once
#include <thread>
#include <atomic>
#include <list>
#include <vector>

#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);

			//void ThreadedUpdate();

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);
			bool SendPacketToPeer(int peer, int msgID);
			bool SendPacketToPeer(int peer, GamePacket& packet);

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			std::vector<ENetPeer*> clients;
			//std::atomic<bool> threadAlive;

			

			//std::thread updateThread;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
