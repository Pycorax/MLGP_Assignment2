#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include "RakNetTypes.h"
#include <map>
#include <vector>
#include <string>
#include "Room.h"

using std::string;
using std::vector;

class RakPeerInterface;

struct GameObject 
{
	GameObject(unsigned int newid)
	: x_(0), y_(0), type_(1)
	{
		id = newid;
	}

	string name;
	unsigned int id;
	float x_;
	float y_;
	int type_;
};

class ServerApp
{
	// TypeDefs
	typedef std::map<SystemAddress, GameObject> ClientMap;

	// Max Users
	static const int MAX_CONNECTIONS = 1;

	// RakNet
	RakPeerInterface* rakpeer_;

	// Client Data Storage
	ClientMap clients_;				// Stores all the clients
	Room lobby;						// Stores references to clients in the lobby
	vector<Room> rooms_;			// Stores references to clients in a room

	unsigned int newID;
	
	void SendWelcomePackage(SystemAddress& addr);
	void SendDisconnectionNotification(SystemAddress& addr);
	void ProcessInitialPosition( SystemAddress& addr, string name, float x_, float y_, int type_);
	void UpdatePosition( SystemAddress& addr, float x_, float y_ );

	// Notification Functions
	// -- These functions update the clients on the server state
	void NotifyServerFull(SystemAddress& addr);
	void NotifyNewRoomCreated();

public:
	ServerApp();
	~ServerApp();
	void Loop();
};

#endif