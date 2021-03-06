#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ship.h"
#include "missile.h"
#include <vector>
#include "Button.h"
#include "RoomButton.h"
#include "MovementServer\Room.h"
#include "ClientGoal.h"
#include "GUIConsole.h"

class HGE;
class hgeFont;
class hgeSprite;
class RakPeerInterface;

//! The default angular velocity of the ship when it is in motion
static const float DEFAULT_ANGULAR_VELOCITY = 3.0f; 
//! The default acceleration of the ship when powered
static const float DEFAULT_ACCELERATION = 50.0f;

/**
* The application class is the main body of the program. It will
* create an instance of the graphics engine and execute the
* Update/Render cycle.
*
*/

class Application
{
	enum APP_STATE
	{
		AS_JOIN,
		AS_LOBBY,
		AS_NEWROOM,
		AS_GAME,
		AS_TOTAL
	};

	enum TEXTURE_TYPE
	{
		TT_BG,
		TT_BOOM,
		TT_BUTTON,
		TT_GOAL,
		TT_SHIP_BLUE,
		TT_SHIP_RED,
		TT_CONSOLE_BG,
		TT_TOTAL
	};

	enum SPRITE_TYPE
	{
		ST_BG,
		ST_BOOM,
		ST_BUTTON,
		ST_GOAL,
		ST_SHIP_BLUE,
		ST_SHIP_RED,
		ST_CONSOLE_BG,
		ST_TOTAL
	};

	enum BUTTON_TYPE
	{
		BT_NEWROOM,
		BT_QUIT,
		BT_TOTAL
	};

	enum INPUT_BUFFER_TYPE
	{
		IBT_NORMAL,
		IBT_CONSOLE,
		IBT_TOTAL
	};

	static const float HEAL_RATE;

	/*
	 * Application
	 */
	HGE* hge_; //!< Instance of the internal graphics engine
	hgeFont* font_;
	HTEXTURE textures_[TT_TOTAL];
	hgeSprite* sprites_[ST_TOTAL];
	APP_STATE appstate;
	
	// Console
	GUIConsole console;
	bool consoleOpen;

	/*
	 * Network
	 */
	RakPeerInterface* rakpeer_;
	unsigned int timer_;
	const int TICK_RATE = 24;
	const int NETWORK_UPDATE_DELTA = 1000 / TICK_RATE;
	bool connecting;

	/*
	 * Lobby Render Positioning
	 */
	static const int ROOM_Y_TITLE_OFFSET = 50;
	static const int ROOM_EACH_Y_OFFSET = 30;
	int roomHeaderYPos;

	/*
	 * Input
	 */
	string inputBuffer[IBT_TOTAL];
	string notifyMessage;		// This string is used to display notifications to the user on any scene.


	/*
	 * Gameplay
	 */
	// Ships
	typedef std::vector<Ship*> ShipList;	//!< A list of ships
	ShipList ships_;						//!< List of all the ships in the universe
	vector<Room*> roomsList;					// List of rooms
	
	// Missiles
	Missile* mymissile;
	bool have_missile;
	typedef std::vector<Missile*> MissileList;
	MissileList missiles_;

	// Goal
	vector<ClientGoal*> goalList;

	// Rooms
	Room* currentRoom;			// Stores a pointer to the current room we are in. If nullptr, means is in lobby
	bool joiningRoom;
	bool leavingRoom;

	// Buttons
	Button buttons[BT_TOTAL];
	vector<RoomButton> roomButtons;

	// App Lifetime
	bool Init();
	static bool Loop();
	void Shutdown();
	bool ControlUpdate(double dt);
	void changeState(APP_STATE state);

	// -- Packet Handling
	int HandlePackets(Packet* packet);

	// -- State Updates
	bool joinUpdate();
	bool lobbyUpdate();
	bool newRoomUpdate();
	bool gameUpdate();

	// -- State Renders
	void joinRender();
	void lobbyRender();
	bool newRoomRender();
	void gameRender();

	// Ships
	Ship* findShip(int id);

	// Rooms
	void createRoomButton(Room* rm);
	void joinRoom(int roomID);
	void leaveRoom();
	Room* findRoom(int roomID);
	Room * findRoomUserIsIn(int userID);

	// Others
	bool updateInputBuffer(int maxBufferLength, INPUT_BUFFER_TYPE type = IBT_NORMAL);
	bool checkCollisions(Ship* ship, ShipList shipsToCheckWith);
	bool SendInitialPosition();

	// Missile Functions
	void CreateMissile( float x, float y, float w, int id );	

public:
	Application();
	~Application() throw();

	void Start();
	bool Update();
	void Render();
};

#endif