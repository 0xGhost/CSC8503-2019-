#pragma once
#include "..\CSC8503Common\NetworkBase.h"
#include "../CSC8503Common/GameObject.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using std::string;
using std::vector;

struct MapPacket : public GamePacket {
	int	mapData[900];
	int x;
	int y;
	MapPacket(const int* map,const int& xx, const int& yy) {
		type = BasicNetworkMessages::Map;
		x = xx;
		y = yy;
		memcpy(mapData, map, x * y * sizeof(int));
		size = sizeof(int) * 902;
	};
};

struct TimePacket : public GamePacket {
	int	time;
	TimePacket(const int& t) {
		type = BasicNetworkMessages::Time;
		time = t;
		size = sizeof(int);
	};
};

struct ObstanclePacket : public GamePacket {
	Vector3 position;
	int score;
	ObstanclePacket(const Vector3& pos, const int& s) {
		type = BasicNetworkMessages::Obstancle;
		position = pos;
		score = s;
		size = sizeof(Vector3) + sizeof(int);
	}
};

struct FinalScorePacket : public GamePacket {
	int ID;
	int score;
	FinalScorePacket(const int& id, const int& s) {
		type = BasicNetworkMessages::FinalScore;
		ID = id;
		score = s;
		size = sizeof(int) * 2;
	}
};

class GamePacketReceiver :
	public PacketReceiver
{
public:
	GamePacketReceiver(string name) {
		this->name = name;
		mapData = nullptr;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		//if(type ==)
		if (type == String_Message)
		{
			StringPacket* realPacket = (StringPacket*)payload;
			string msg = realPacket->GetStringFromData();
			stringMessages.push_back(msg);
			//std::cout << name << "received message: " << msg << std::endl;
			return;
		}
		if (type == Player_Connected)
		{
			NewPlayerPacket* realPacket = (NewPlayerPacket*)payload;
			playersID.push_back(realPacket->playerID);
			return;
		}
		if (type == Player_Disconnected)
		{
			PlayerDisconnectPacket* realPacket = (PlayerDisconnectPacket*)payload;
			playersID.erase(std::remove(playersID.begin(), playersID.end(), realPacket->playerID), playersID.end());
			return;
		}
		if (type == Map)
		{
			MapPacket* realPacket = (MapPacket*)payload;
			mapX = realPacket->x;
			mapY = realPacket->y;
			mapData = new int[mapX * mapY];
			memcpy(mapData, realPacket->mapData, mapX * mapY * sizeof(int));
			mapReady = true;
			return;
		}
		if (type == Time)
		{
			TimePacket* realPacket = (TimePacket*)payload;
			time = realPacket->time;
			return;
		}
		if (type == Obstancle)
		{
			ObstanclePacket* realPacket = (ObstanclePacket*)payload;
			obstanclePosition = realPacket->position;
			obstancleScore = realPacket->score;
			obstancleReady = true;
			return;
		}
		if (type == FinalScore)
		{
			FinalScorePacket* realPacket = (FinalScorePacket*)payload;
			finalScoreMessage += " Player" + std::to_string(realPacket->ID) + ":" + std::to_string(realPacket->score);
		}
	}

	int GetTime() { return time; }

	string GetFinalScoreMessage() { return finalScoreMessage; }
	
	bool ObstancleReady() {
		if (obstancleReady)
		{
			obstancleReady = false;
			return true;
		}
		return false;
	}

	bool MapReady() { 
		if(mapReady)
		{
			mapReady = false;
			return true;
		}
		return false;
	}

	void GetObstancleData(Vector3& pos, int& score)
	{
		pos = obstanclePosition;
		score = obstancleScore;
	}

	void LoadMapData(int& x, int& y, int*& mapPtr)
	{
		
		mapPtr = mapData;
		x = this->mapX;
		y = this->mapY;
	}

	vector<int> GetPlayers()
	{
		return playersID;
	}
	string GetNextString()
	{
		string s;
		if (!stringMessages.empty())
		{
			s = stringMessages.back();
			stringMessages.pop_back();
		}
		return s;
	}
	void ClearStringMessages()
	{
		stringMessages.clear();
	}

protected:
	string name;
	vector<string> stringMessages;
	vector<int> playersID;
	int mapX, mapY;
	int* mapData;
	bool mapReady;
	bool obstancleReady;
	int time;
	Vector3 obstanclePosition;
	int obstancleScore;
	string finalScoreMessage;
	//vector<Vector3> positionMessages;
};

