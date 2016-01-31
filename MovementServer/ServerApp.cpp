#include "ServerApp.h"

// STL Includes
#include <iostream>

// API Includes
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"
#include "../MyMsgIDs.h"

// Other Includes
#include "../ship.h"

// Using Directives
using std::to_string;

void ServerApp::NotifyServerFull(SystemAddress & addr)
{
	RakNet::BitStream bs;

	// State the purpose of the message
	bs.Write(static_cast<unsigned char>(ID_SERVER_FULL));

	// Broadcast to everyone that this new room has been created
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);

	console->Print("User attemped to join full server!\n");
}

void ServerApp::NotifyNewRoomCreated()
{
	RakNet::BitStream bs;

	// State the purpose of the message
	bs.Write(static_cast<unsigned char>(ID_NEWROOM));

	// Send details of the new room
	// --Send the room name
	bs.Write(rooms_.back().GetName().c_str());
	// -- Send the room ID
	bs.Write(rooms_.back().GetID());

	// Broadcast to everyone that this new room has been created
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	console->Print("Created new room: " + rooms_.back().GetName() + " of ID #" + to_string(rooms_.back().GetID()) + "!" + to_string(static_cast<unsigned char>(ID_NEWROOM)) + "\n");
}

void ServerApp::NotifyUserJoinedRoom(SystemAddress & userThatJoined, int roomJoined)
{
	RakNet::BitStream bs;

	// State the purpose of the message
	bs.Write(static_cast<unsigned char>(ID_JOINROOM));

	// Send details of the situation
	// -- The ID of the person that joined the room
	bs.Write(clients_.at(userThatJoined).id);
	// -- The room that the person joined
	bs.Write(roomJoined);
	// Broadcast to everyone that this new room has been created
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

bool ServerApp::userIsInARoom(int userID)
{
	auto userLobbyList = lobby.GetConnectedIDs();

	for (auto user : userLobbyList)
	{
		if (user == userID)
		{
			return false;
		}
	}

	return true;
}

Room* ServerApp::findRoom(int roomID)
{
	for (auto& room : rooms_)
	{
		if (room.GetID() == roomID)
		{
			return &room;
		}
	}

	return nullptr;
}

ServerApp::ServerApp(float packetHandlerDelay) 
	: rakpeer_(RakNetworkFactory::GetRakPeerInterface())
	, newID(0)
	, console(Console::Instance())
{
	rakpeer_->Startup(100, 30, &SocketDescriptor(1691, 0), 1);
	rakpeer_->SetMaximumIncomingConnections(MAX_CONNECTIONS);
	rakpeer_->SetOccasionalPing(true);
	loopDelay[THREAD_PACKET_HANDLER] = packetHandlerDelay;
	console->Print("Server Started\n");
}

ServerApp::~ServerApp()
{
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

void ServerApp::PacketHandlerLoop()
{
	Sleep(loopDelay[THREAD_PACKET_HANDLER]);

	if (Packet* packet = rakpeer_->Receive())
	{
		RakNet::BitStream bs(packet->data, packet->length, false);
		
		unsigned char msgid = 0;
		RakNetTime timestamp = 0;

		bs.Read(msgid);

		if (msgid == ID_TIMESTAMP)
		{
			bs.Read(timestamp);
			bs.Read(msgid);
		}

		switch (msgid)
		{
		case ID_NEW_INCOMING_CONNECTION:
			if (clients_.size() < MAX_CONNECTIONS)
			{
				SendWelcomePackage(packet->systemAddress);
			}
			else
			{
				NotifyServerFull(packet->systemAddress);
			}
			break;

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			SendDisconnectionNotification(packet->systemAddress);
			break;

		case ID_INITIALPOS:
			{
				float x_, y_;
				int type_;
				char name_[Ship::MAX_NAME_LENGTH];
				console->Print("ProcessInitialPosition\n");
				bs.Read(name_);
				bs.Read( x_ );
				bs.Read( y_ );
				bs.Read( type_ );
				ProcessInitialPosition( packet->systemAddress, name_, x_, y_, type_);
			}
			break;
#pragma region Room Management Messages

		case ID_NEWROOM:
			{
				// Receive the room name
				char roomName[Room::MAX_ROOM_NAME_LENGTH];
				bs.Read(roomName);
				// Create the room
				rooms_.push_back(Room(roomName, rooms_.size()));

				// Inform everyone of the new room
				NotifyNewRoomCreated();
			}
			break;

		case ID_JOINROOM:
			{
				// Receive the room ID to join
				int roomID = -1;
				bs.Read(roomID);
				// Check if this user is in a room
				int userID = clients_.at(packet->systemAddress).id;
				if (!userIsInARoom(userID))
				{
					// Since he is not in a room, let's let him join the room
					Room* rm = findRoom(roomID);

					// If we found the room
					if (rm != nullptr)
					{
						// Add the User
						rm->AddUser(userID);
						// Notify the user that he has joined and everyone that someone has entered this room
						NotifyUserJoinedRoom(packet->systemAddress, roomID);
						// Print on Server
						console->Print("User #" + to_string(userID) + " has joined room #" + to_string(roomID) + ": " + rm->GetName() + "!\n");
					}
					else
					{
						console->Print("User #" + to_string(userID) + " tried to join non-existent room #" + to_string(roomID) + "!\n");
					}
				}
				else
				{
					console->Print("User #" + to_string(userID) + " tried to join room #" + to_string(roomID) + " while in a room.!\n");
				}
			}
			break;

#pragma endregion

	#pragma region Game Messages
		case ID_MOVEMENT:
			{
				float x, y;
				unsigned int shipid;
				bs.Read(shipid);
				bs.Read(x);
				bs.Read(y);
				UpdatePosition( packet->systemAddress, x, y );

				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			}
			break;

		case ID_COLLIDE:
		case ID_INJURED:
			{
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			}
			break;

		// Lab 13 Task 14 : new cases on server side to handle missiles
		case ID_NEWMISSILE:
			{
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			}
			break;

		case ID_UPDATEMISSILE:
		{
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		}
		break;
	#pragma endregion

		default:
			console->Print("Unhandled Message Identifier: " + to_string((int)msgid) + "\n");
		}

		rakpeer_->DeallocatePacket(packet);
	}
}

void ServerApp::ConsoleLoop()
{
	console->Update();
}

void ServerApp::SendWelcomePackage(SystemAddress& addr)
{
	++newID;
	unsigned int shipcount = static_cast<unsigned int>(clients_.size());
	unsigned char msgid = ID_WELCOME;
	
	RakNet::BitStream bs;
	// Send ship ID
	bs.Write(msgid);
	bs.Write(newID);
	bs.Write(shipcount);

	// Send list of ships
	for (ClientMap::iterator itr = clients_.begin(); itr != clients_.end(); ++itr)
	{
		console->Print("Ship " + itr->second.name + " (" + to_string(itr->second.id) + ") pos" + to_string(itr->second.x_) + " " + to_string(itr->second.y_) + "\n");
		bs.Write( itr->second.name.c_str());
		bs.Write( itr->second.id );
		bs.Write( itr->second.x_ );
		bs.Write( itr->second.y_ );
		bs.Write( itr->second.type_ );
	}

	// Send list of rooms and members
	// -- Send the number of rooms so that the client knows what to expect
	bs.Write(rooms_.size());

	// -- Send each room
	for (auto room : rooms_)
	{
		// Send the room name
		bs.Write(room.GetName().c_str());

		// Get the list of users
		auto connectedIDs = room.GetConnectedIDs();

		// Send the number of users so that the client knows what to expect
		bs.Write(connectedIDs.size());

		// Send the list of users
		for (auto id : connectedIDs)
		{
			bs.Write(id);
		}
	}

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED,0, addr, false);

	bs.Reset();

	GameObject newobject(newID);

	clients_.insert(std::make_pair(addr, newobject));

	console->Print("New guy, assigned id " + to_string(newID) + "!\n");
}

void ServerApp::SendDisconnectionNotification(SystemAddress& addr)
{
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	unsigned char msgid = ID_LOSTSHIP;
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(itr->second.id);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);

	console->Print(to_string(itr->second.id) + " has left the game" + "!\n");

	clients_.erase(itr);

}

void ServerApp::ProcessInitialPosition( SystemAddress& addr, string name_, float x_, float y_, int type_)
{
	unsigned char msgid;
	RakNet::BitStream bs;
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	itr->second.name = name_;
	itr->second.x_ = x_;
	itr->second.y_ = y_;
	itr->second.type_ = type_;
	
	console->Print("Player " + itr->second.name + " connected." + "\n");
	console->Print("Received pos" + to_string(itr->second.x_) + " " + to_string(itr->second.y_) + "\n");
	console->Print("Received type" + to_string(itr->second.type_) + "\n");

	msgid = ID_NEWSHIP;
	bs.Write(msgid);
	bs.Write(itr->second.id);
	bs.Write(itr->second.name.c_str());
	bs.Write(itr->second.x_);
	bs.Write(itr->second.y_);
	bs.Write(itr->second.type_);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);

	// Add this ship to the lobby
	lobby.AddUser(itr->second.id);
}

void ServerApp::UpdatePosition( SystemAddress& addr, float x_, float y_ )
{
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	itr->second.x_ = x_;
	itr->second.y_ = y_;
}