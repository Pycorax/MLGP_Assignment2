#ifndef SERVERAPP_H_
#define SERVERAPP_H_

// STL Includes
#include <map>
#include <vector>
#include <string>

// API Includes
#include "RakNetTypes.h"
#include <Windows.h>

// Other Includes
#include "Room.h"
#include "Console.h"
#include "Goal.h"

// Using Directives
using std::string;
using std::vector;

class RakPeerInterface;

struct GameObject 
{
	GameObject(unsigned int newid)
	: x_(0), y_(0)
	{
		id = newid;
	}

	string name;
	unsigned int id;
	float x_;
	float y_;
};

class ServerApp
{
public:
	enum THREAD_TYPE
	{
		THREAD_PACKET_HANDLER,
		THREAD_CONSOLE,
		THREAD_GAME,
		THREAD_TOTAL
	};

private:
	// TypeDefs
	typedef std::map<SystemAddress, GameObject> ClientMap;

	// Loop Delays (Prevent code from executing too fast)
	float loopDelay[THREAD_TOTAL];

	// User Handling
	static const int MAX_CONNECTIONS = 10;
	unsigned int newID;

	// RakNet
	RakPeerInterface* rakpeer_;

	// Client Data Storage
	ClientMap clients_;				// Stores all the clients
	Room lobby;						// Stores references to clients in the lobby
	vector<Room> rooms_;			// Stores references to clients in a room

	// Goals. The same goals will be used for all rooms.
	Goal leftGoal;
	Goal rightGoal;

	// Handle to the Console
	Console* console;

	// For Delta Time
	double oldTime;

	// Multi-Threading
	CRITICAL_SECTION newRoomCSection;
	CRITICAL_SECTION roomAccessCSection;

	void SendWelcomePackage(SystemAddress& addr);
	void SendDisconnectionNotification(SystemAddress& addr);
	void ProcessInitialPosition( SystemAddress& addr, string name, float x_, float y_, int type_);
	void UpdatePosition( SystemAddress& addr, float x_, float y_ );

	// Notification Functions
	// -- These functions update the clients on the server state
	void NotifyServerFull(SystemAddress& addr);
	void NotifyNewRoomCreated();
	void NotifyUserJoinedRoom(SystemAddress& userThatJoined, int roomJoined);

	// Rooms Helper Functions   ---> In hindsight, I should've really put this into a RoomManager class but too late now
	// -- Create a room
	void createRoom(string roomName);
	// -- Function to check if the user is in a room
	bool userIsInARoom(int userID);
	// -- Function to find the room that a user is in
	Room* findRoomUserIsIn(int userID);
	// -- Get pointer to a room via the ID
	Room* findRoom(int roomID);

public:
	ServerApp(float packetHandlerDelay = 10.0f, float consoleDelay = 100.0f, float gameDelay = 0.0f);
	~ServerApp();

	void PacketHandlerLoop();
	void ConsoleLoop();
	void GameLoop();
};

#endif