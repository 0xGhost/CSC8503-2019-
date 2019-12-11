#pragma once
#include "..\CSC8503Common\NetworkBase.h"
#include <iostream>
using std::string;

class GamePacketReceiver :
	public PacketReceiver
{
public:
	GamePacketReceiver(string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;


			string msg = realPacket->GetStringFromData();

			std::cout << name << "received message: " << msg << std::endl;
		}

	}
protected:
	string name;
};

