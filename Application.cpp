#include "Application.h"
#include "ship.h"
#include "Globals.h"
#include "MyMsgIDs.h"
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"
#include <hge.h>
#include <string>
#include <iostream>
#include <fstream>
#include <hgefont.h>

// Lab 13 Task 9a : Uncomment the macro NETWORKMISSILE
#define NETWORKMISSILE

float GetAbsoluteMag( float num )
{
	if ( num < 0 )
	{
		return -num;
	}

	return num;
}

const float Application::HEAL_RATE = 1000.0f;

/** 
* Constuctor
*
* Creates an instance of the graphics engine and network engine
*/

Application::Application()
	: hge_(hgeCreate(HGE_VERSION))
	, appstate(AS_JOIN)
	, rakpeer_(RakNetworkFactory::GetRakPeerInterface())
	, connecting(false)
	, timer_(0)
	// Lab 13 Task 2 : add new initializations
	, mymissile(nullptr)
	, joiningRoom(false)
	, leavingRoom(false)
	, currentRoom(nullptr)
	, consoleOpen(false)
{
}

/**
* Destructor
*
* Does nothing in particular apart from calling Shutdown
*/

Application::~Application() throw()
{
	Shutdown();
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

/**
* Initialises the graphics system
* It should also initialise the network system
*/

bool Application::Init()
{
	// Seed Random
	srand( RakNet::GetTime() );

	// Set up HGE variables
	hge_->System_SetState(HGE_FRAMEFUNC, Application::Loop);
	hge_->System_SetState(HGE_WINDOWED, true);
	hge_->System_SetState(HGE_USESOUND, false);
	hge_->System_SetState(HGE_HIDEMOUSE, false);
	hge_->System_SetState(HGE_TITLE, "Multiplayer Game Programming Assignment 2");
	hge_->System_SetState(HGE_SCREENWIDTH, 800);
	hge_->System_SetState(HGE_SCREENHEIGHT, 600);
	hge_->System_SetState(HGE_LOGFILE, "movement.log");
	hge_->System_SetState(HGE_DONTSUSPEND, true);

	// Attempt to start up HGE
	if(hge_->System_Initiate()) 
	{
		// Get the screen resolution
		float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
		float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

		// Load Textures
		textures_[TT_BG] = hge_->Texture_Load("background.png");
		textures_[TT_BOOM] = hge_->Texture_Load("boom.png");
		textures_[TT_BUTTON] = hge_->Texture_Load("button.png");
		textures_[TT_GOAL] = hge_->Texture_Load("goal.png");
		textures_[TT_SHIP_BLUE] = hge_->Texture_Load("ship_blue.png");
		textures_[TT_SHIP_RED] = hge_->Texture_Load("ship_red.png");
		textures_[TT_CONSOLE_BG] = hge_->Texture_Load("consoleBG.png");

		// Load Sprites
		sprites_[ST_BG] = new hgeSprite(textures_[TT_BG], 0, 0, screenwidth, screenheight);
		sprites_[ST_BG]->SetHotSpot(screenwidth * 0.5f, screenheight * 0.5f);
		sprites_[ST_BOOM] = new hgeSprite(textures_[TT_BOOM], 0, 0, 40, 40);
		sprites_[ST_BOOM]->SetHotSpot(20, 20);
		sprites_[ST_BUTTON] = new hgeSprite(textures_[TT_BUTTON], 0, 0, 250, 50);
		sprites_[ST_BUTTON]->SetHotSpot(125, 25);
		sprites_[ST_GOAL] = new hgeSprite(textures_[TT_GOAL], 0, 0, 50, 200);
		sprites_[ST_GOAL]->SetHotSpot(25, 100); 
		sprites_[ST_SHIP_BLUE] = new hgeSprite(textures_[TT_SHIP_BLUE], 0, 0, 64, 64);
		sprites_[ST_SHIP_BLUE]->SetHotSpot(32, 32);
		sprites_[ST_SHIP_RED] = new hgeSprite(textures_[TT_SHIP_RED], 0, 0, 64, 64);
		sprites_[ST_SHIP_RED]->SetHotSpot(32, 32);
		sprites_[ST_CONSOLE_BG] = new hgeSprite(textures_[TT_CONSOLE_BG], 0, 0, 20, 20);
		sprites_[ST_CONSOLE_BG]->SetHotSpot(10, 10);

		// Load Fonts
		font_ = new hgeFont("font1.fnt");
		font_->SetScale(0.5);

		// Set up the Buttons
		buttons[BT_QUIT].Init(sprites_[ST_BUTTON], font_, screenwidth * 0.7f, screenheight * 0.9f, 250, 50, "Quit");
		buttons[BT_NEWROOM].Init(sprites_[ST_BUTTON], font_, screenwidth * 0.3f, screenheight * 0.9f, 250, 50, "New Room");

		// Define the start Y position that the room header should be printed from
		roomHeaderYPos = screenheight * 0.2f;

		// Initialize the Console
		console.Init(10, screenwidth, sprites_[ST_CONSOLE_BG], font_);

		// Attempt to start up RakNet
		if (rakpeer_->Startup(1,30,&SocketDescriptor(), 1))
		{
			// Init RakNet settings
			rakpeer_->SetOccasionalPing(true);

			return true;
		}
	}

	return false;
}

/**
* Update cycle
*
* Checks for keypresses:
*   - Esc - Quits the game
*   - Left - Rotates ship left
*   - Right - Rotates ship right
*   - Up - Accelerates the ship
*   - Down - Deccelerates the ship
*
* Also calls Update() on all the ships in the universe
*/
bool Application::Update()
{
	// Console Open/Close
	static const double TOGGLE_TIME_DELAY = 0.1;
	static double toggleTimer = 0.0;
	toggleTimer += hge_->Timer_GetDelta();
	if (hge_->Input_GetKeyState(HGEK_GRAVE) && toggleTimer > TOGGLE_TIME_DELAY)
	{
		// Toggle Console
		consoleOpen = !consoleOpen;

		// Reset toggle timer
		toggleTimer = 0.0;
	}

	// Console Update
	if (consoleOpen)
	{
		bool enter = updateInputBuffer(GUIConsole::MAX_COMMAND_LENGTH, IBT_CONSOLE);
		console.Update(enter, inputBuffer[IBT_CONSOLE]);

		if (enter)
		{
			inputBuffer[IBT_CONSOLE] = "";

			// Send command to server
			RakNet::BitStream bs;
			bs.Write(static_cast<unsigned char>(MyMsgIDs::ID_COMMAND));
			bs.Write(console.GetLastMessage().c_str());
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

			std::cout << "Send command: " << console.GetLastMessage().c_str() << " to the server!" << std::endl;
		}
	}

	switch (appstate)
	{
		case AS_JOIN:
			return joinUpdate();
		case AS_LOBBY:
			return lobbyUpdate();
		case AS_NEWROOM:
			return newRoomUpdate();
		case AS_GAME:
			return gameUpdate();
	}
}


/**
* Render Cycle
*
* Clear the screen and render all the ships
*/
void Application::Render()
{
	hge_->Gfx_BeginScene();
	hge_->Gfx_Clear(0);

	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	switch (appstate)
	{
		case AS_JOIN:
			joinRender();
			break;
		case AS_LOBBY:
			lobbyRender();
			break;
		case AS_NEWROOM:
			newRoomRender();
			break;
		case AS_GAME:
			gameRender();
			break;
	}

	if (consoleOpen)
	{
		console.Render(0, screenheight);
	}

	hge_->Gfx_EndScene();
}


/** 
* Main game loop
*
* Processes user input events
* Supposed to process network events
* Renders the ships
*
* This is a static function that is called by the graphics
* engine every frame, hence the need to loop through the
* global namespace to find itself.
*/
bool Application::Loop()
{
	Global::application->Render();
	return Global::application->Update();
}

/**
* Shuts down the graphics and network system
*/

void Application::Shutdown()
{
	// Clear textures
	for (size_t i = 0; i < TT_TOTAL; ++i)
	{
		hge_->Texture_Free(textures_[i]);
	}

	// Clear sprites
	for (size_t i = 0; i < ST_TOTAL; ++i)
	{
		delete sprites_[i];
		sprites_[i] = nullptr;
	}

	// Clear all rooms
	while (roomsList.size() > 0)
	{
		delete roomsList.back();
		roomsList.pop_back();
	}

	hge_->System_Shutdown();
	hge_->Release();
}

bool Application::ControlUpdate(double dt)
{
	if (hge_->Input_GetKeyState(HGEK_ESCAPE))
	{
		return true;
	}

	if (hge_->Input_GetKeyState(HGEK_LEFT))
	{
		ships_.at(0)->SetAngularVelocity(ships_.at(0)->GetAngularVelocity() - DEFAULT_ANGULAR_VELOCITY);
	}

	if (hge_->Input_GetKeyState(HGEK_RIGHT))
	{
		ships_.at(0)->SetAngularVelocity(ships_.at(0)->GetAngularVelocity() + DEFAULT_ANGULAR_VELOCITY);
	}

	if (hge_->Input_GetKeyState(HGEK_UP))
	{
		ships_.at(0)->Accelerate(DEFAULT_ACCELERATION, dt);
	}

	if (hge_->Input_GetKeyState(HGEK_DOWN))
	{
		ships_.at(0)->Accelerate(-DEFAULT_ACCELERATION, dt);
	}

	// Lab 13 Task 4 : Add a key to shoot missiles
	static const double SHOT_DELAY = 0.5;
	static double shotTimer = 0.0;
	shotTimer += hge_->Timer_GetDelta();
	if (hge_->Input_GetKeyState(HGEK_SPACE) && shotTimer > SHOT_DELAY)
	{
		CreateMissile(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID());
		shotTimer = 0.0;
	}	

	return false;
}

void Application::changeState(APP_STATE state)
{
	appstate = state;
	// Reset the notify message
	notifyMessage = "";

	// Reset the input buffer
	inputBuffer[IBT_NORMAL] = "";
}

int Application::HandlePackets(Packet * packet)
{
	if (packet)
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
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Connected to Server" << std::endl;
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
		case ID_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "Lost Connection to Server" << std::endl;
			
			if (appstate == AS_JOIN)
			{
				notifyMessage = "Server is FULL!";
				connecting = false;
			}

			break;

		case ID_WELCOME:
		{
			unsigned int shipcount, id;
			float x_, y_;
			char chartemp[5];
			char nameArray[Ship::MAX_NAME_LENGTH];

			bs.Read(id);
			ships_.at(0)->setID(id);
			ships_.at(0)->SetBoomSprite(sprites_[ST_BOOM]);
			bs.Read(shipcount);
			
			// Receive list of all ships
			for (unsigned int i = 0; i < shipcount; ++i)
			{
				bs.Read(nameArray);
				bs.Read(id);
				bs.Read(x_);
				bs.Read(y_);
				std::cout << "New Ship " << nameArray << " at pos" << x_ << " " << y_ << std::endl;
				Ship* ship = new Ship(x_, y_);
				ship->SetName(nameArray);
				ship->SetBoomSprite(sprites_[ST_BOOM]);
				ship->setID(id);
				ships_.push_back(ship);
			}

			// Receive goal data
			// -- Define variables for reading
			int numGoals = 0;
			// -- Receive number of goals
			bs.Read(numGoals);
			for (int i = 0; i < numGoals; ++i)
			{
				goalList.push_back(new ClientGoal);
				goalList.back()->RecvObject(&bs, ID_WELCOME);
				goalList.back()->SetSprite(sprites_[ST_GOAL]);
			}

			// Receive rooms data
			// -- Define variables for reading
			char roomName[Room::MAX_ROOM_NAME_LENGTH];
			int roomID = -1;
			int numShips = 0;
			int shipID = 0;
			Room::TEAM_TYPE shipTeam;
			// -- Get number of rooms
			unsigned numRooms = 0;
			bs.Read(numRooms);

			// -- Iterate through the rooms we expect
			for (unsigned int i = 0; i < numRooms; ++i)
			{
				// Initialize each room
				// -- Get the room name
				bs.Read(roomName);
				// -- Get the room ID
				bs.Read(roomID);

				// -- Create the room
				roomsList.push_back(new Room(roomName, roomID));

				// -- Create a button for that room
				createRoomButton(roomsList.at(i));

				// -- Get the number of ships in this room
				bs.Read(numShips);

				// -- Receive the IDs of ships in this room
				for (unsigned int ship = 0; ship < numShips; ++ship)
				{
					// Retrieve the ship ID
					bs.Read(shipID);
					// Retrieve the ship team
					bs.Read(shipTeam);

					// Add the ID in
					roomsList.back()->AddUser(shipID, shipTeam);
				}

				std::cout << "New Room: " << roomName << " with " << numShips << " players." << std::endl;
			}

			SendInitialPosition();
		}
		break;

	#pragma region Room Messages

		case ID_NEWROOM:
		{
			// Get the room name
			char roomName[Room::MAX_ROOM_NAME_LENGTH];
			bs.Read(roomName);
			// Get the room ID
			int roomID = 0;
			bs.Read(roomID);

			// Create the room
			roomsList.push_back(new Room(roomName, roomID));

			// Create the button for the room
			createRoomButton(roomsList.back());

			std::cout << "New Room Created on Server: " << roomName << " #" << roomID << std::endl;
		}
		break;

		case ID_JOINROOM:
		{
			// Get the user who joined
			int userID;
			bs.Read(userID);
			// Get the room ID
			int roomID = 0;
			bs.Read(roomID);
			// Get the team type
			Room::TEAM_TYPE team = Room::TOTAL_TEAMS;
			bs.Read(team);
			
			// Update the room
			// -- Find the room
			Room* rm = findRoom(roomID);
			if (rm)
			{
				// Add the user into the room to update the room
				rm->AddUser(userID, team);

				// Check if we are the one getting the join message
				if (userID == ships_.at(0)->GetID())
				{
					// Set the current room
					currentRoom = rm;
					// We have joined!, we can trying stop now.
					joiningRoom = false;
					// Go to the game
					changeState(AS_GAME);

					std::cout << "Joined room #" << roomID << "!" << std::endl;

					// If we are the ones who have joined, change the sprites of all in the room
					auto connectedIDS = rm->GetConnectedIDs();
					for (auto id : connectedIDS)
					{
						Ship* ship = findShip(id);
						if (rm->GetUserTeam(id) == Room::TEAM_BLUE)
						{
							ship->SetSprite(sprites_[ST_SHIP_BLUE]);
						}
						else
						{
							ship->SetSprite(sprites_[ST_SHIP_RED]);
						}
					}
					
				}
				else
				{
					std::cout << "User #" << userID << " has joined room #" << roomID << "!" << std::endl;

					Ship* ship = findShip(userID);
					// If this is a new user joining, change their sprite to the correct one
					if (rm->GetUserTeam(userID) == Room::TEAM_BLUE)
					{
						ship->SetSprite(sprites_[ST_SHIP_BLUE]);
					}
					else
					{
						ship->SetSprite(sprites_[ST_SHIP_RED]);
					}
				}

				// Reset the health of the guy who joined, be it u or me
				Ship* joiner = findShip(userID);
				if (joiner)
				{
					joiner->ResetHealth();
				}
			}
		}
		break;

		case ID_LEAVEROOM:
		{
			// Get the user who joined
			int userID;
			bs.Read(userID);

			// Update the room
			// -- Find the room of this user
			Room* rm = findRoomUserIsIn(userID);
			if (rm)
			{
				// Remove the user from the room to update the room
				rm->RemoveUser(userID);

				// Check if we are the one getting the join message
				if (userID == ships_.at(0)->GetID())
				{
					// Set the current room
					currentRoom = nullptr;
					// We have left!, we can trying stop now.
					leavingRoom = false;
					// Go to the game lobby
					changeState(AS_LOBBY);

					std::cout << "Returned to lobby!" << std::endl;
				}
				else
				{
					std::cout << "User #" << userID << " has has returned to lobby!" << std::endl;
				}
			}
		}
		break;

	#pragma endregion

#pragma region Gameplay Messages

		case ID_NEWSHIP:
		{
			unsigned int id;
			bs.Read(id);

			if (id == ships_.at(0)->GetID())
			{
				// if it is me
				break;
			}
			else
			{
				float x_, y_;
				char name_[Ship::MAX_NAME_LENGTH];

				bs.Read(name_);
				bs.Read(x_);
				bs.Read(y_);
				std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
				Ship* ship = new Ship( x_, y_);
				ship->SetName(name_);
				ship->SetBoomSprite(sprites_[ST_BOOM]);
				ship->setID(id);
				ships_.push_back(ship);
			}

		}
		break;

		case ID_LOSTSHIP:
		{
			// Remove the ship that was lost
			unsigned int shipid;
			bs.Read(shipid);
			for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
			{
				if ((*itr)->GetID() == shipid)
				{
					// Remove the ship from any room it might be in
					Room* roomUserIsIn = findRoomUserIsIn(shipid);
					if (roomUserIsIn)
					{
						roomUserIsIn->RemoveUser(shipid);
					}

					// Remove the ship
					delete *itr;
					ships_.erase(itr);
					break;
				}
			}
		}
		break;

		case ID_INITIALPOS:
			break;

		case ID_MOVEMENT:
		{
			unsigned int shipid;
			bs.Read(shipid);

			for (auto itr = ships_.begin(); itr != ships_.end(); ++itr)
			{
				if ((*itr)->GetID() == shipid)
				{
					// Try to receive this object and process it
					(*itr)->RecvObject(&bs, ID_MOVEMENT);
					break;
				}
			}
		}
		break;

		case ID_COLLIDE:
			{
				unsigned int shipid;
				float x, y;
				bs.Read(shipid);

				if (shipid == ships_.at(0)->GetID())
				{
					std::cout << "collided with someone!" << std::endl;

					ships_.at(0)->RecvObject(&bs, ID_COLLIDE);
				}
			}
			break;

		case ID_INJURED:
			{
				unsigned int shipid;
				bs.Read(shipid);

				if (shipid == ships_.at(0)->GetID())
				{
					std::cout << "hit by someone!" << std::endl;

					ships_.at(0)->RecvObject(&bs, ID_INJURED);
				}
			}
			break;

		case ID_HEAL:
			{
				int shipid;
				bs.Read(shipid);

				for (auto& ship : ships_)
				{
					if (ship->GetID() == shipid)
					{
						ship->RecvObject(&bs, ID_HEAL);
						break;
					}
				}
			}
			break;
		case ID_NEWMISSILE:
		{
			float x, y, w;
			int id;

			bs.Read(id);
			bs.Read(x);
			bs.Read(y);
			bs.Read(w);

			missiles_.push_back(new Missile("missile.png", x, y, w, id));
		}
		break;

		case ID_UPDATEMISSILE:
		{
			int id;
			// Get the ID of the missile to update
			bs.Read(id);

			// Find the ID of the missile to update
			for (auto itr = missiles_.begin(); itr != missiles_.end(); ++itr)
			{
				if ((*itr)->GetOwnerID() == id)
				{
					(*itr)->RecvObject(&bs, ID_UPDATEMISSILE);

					// Delete this object if it is marked for deletion
					if ((*itr)->GetDeleted())
					{
						delete *itr;
						missiles_.erase(itr);
					}

					break;
				}

			}
		}
		break;

		case ID_UPDATEGOAL:
		{
			int goalID = -1;
			bs.Read(goalID);

			for (auto goal : goalList)
			{
				// If we are the correct recipient
				if (goalID == goal->GetID())
				{
					goal->RecvObject(&bs, ID_UPDATEGOAL);
					break;
				}
			}
		}
		break;

#pragma endregion

		default:
		{
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;
		}
		break;
		}

		rakpeer_->DeallocatePacket(packet);

		return msgid;
	}

	// No messages
	return -1;
}

bool Application::joinUpdate()
{
	static string shipNameSaved = "";

	if (!consoleOpen && !connecting && updateInputBuffer(Ship::MAX_NAME_LENGTH))
	{
		// Only continue if there is a name specified
		if (inputBuffer[IBT_NORMAL].length() > 0)
		{
			// Save the ship name
			shipNameSaved = inputBuffer[IBT_NORMAL];

			// Reset the input buffer
			inputBuffer[IBT_NORMAL] = "";

			// Connect to the server
			if (!connecting && rakpeer_->Connect("127.0.0.1", 1691, 0, 0))
			{
				connecting = true;
				notifyMessage = "Attempting to connect...";
			}
		}
		else
		{
			notifyMessage = "Please enter a ship name...";
		}
	}

	// If a Welcome Message was received, then go to the lobby
	int msgID = HandlePackets(rakpeer_->Receive());
	switch (msgID)
	{
		case ID_WELCOME:
			// Go to the next state
			changeState(AS_LOBBY);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			connecting = false;
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			// Initialize ships
			ships_.push_back(new Ship(rand() % 500 + 100, rand() % 400 + 100));
			ships_.at(0)->SetName(shipNameSaved.c_str());
			connecting = false;
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			connecting = false;
			// Inform the user that we can't connect
			notifyMessage = "Unable to connect to server!";
			break;
	}

	return false;
}

bool Application::lobbyUpdate()
{
	float mouseXPos, mouseYPos;
	hge_->Input_GetMousePos(&mouseXPos, &mouseYPos);

	// Button Handling
	for (auto& button : buttons)
	{
		button.Update(mouseXPos, mouseYPos, hge_->Input_GetKeyState(HGEK_LBUTTON));
	}

	if (buttons[BT_QUIT].GetState())
	{
		return true;
	}
	else if (buttons[BT_NEWROOM].GetState())
	{
		changeState(AS_NEWROOM);
	}

	if (!joiningRoom)
	{
		for (auto& rb : roomButtons)
		{
			rb.Update(mouseXPos, mouseYPos, hge_->Input_GetKeyState(HGEK_LBUTTON));

			if (rb.GetState())
			{
				// Request to enter a room
				joinRoom(rb.GetRoomID());
				notifyMessage = "Attempting to join the room...";
				break;
			}
		}
	}

	// Packet Handling
	HandlePackets(rakpeer_->Receive());

	return false;
}

bool Application::newRoomUpdate()
{
	// Once the user has finished typing,
	if (!consoleOpen && updateInputBuffer(Room::MAX_ROOM_NAME_LENGTH))
	{
		// Tell the server to create this room
		// -- Create BitStream object
		RakNet::BitStream bs;

		// State the message type
		bs.Write(static_cast<unsigned char>(ID_NEWROOM));

		// Ship Update
		bs.Write(inputBuffer[IBT_NORMAL].c_str());

		// Send this message
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		// Go back to the lobby
		changeState(AS_LOBBY);
	}

	// If cancel creating new room
	if (hge_->Input_GetKeyState(HGEK_ESCAPE))
	{
		// Go back to the lobby
		changeState(AS_LOBBY);
	}

	return false;
}

bool Application::gameUpdate()
{
	// Exit
	if (!leavingRoom && hge_->Input_GetKeyState(HGEK_ESCAPE))
	{
		// Send message requesting leave room
		leaveRoom();
	}

	// Get the delta time
	float timedelta = hge_->Timer_GetDelta();

	// Reset my angular velocity
	ships_.at(0)->SetAngularVelocity(0.0f);

	// Get Controls
	ControlUpdate(timedelta);

	// Get a list of ships in this room
	ShipList roomShips;
	auto shipsInRoom = currentRoom->GetConnectedIDs();
	for (auto ship : ships_)
	{
		// Check if this ship is in the room
		for (auto s : shipsInRoom)
		{
			if (ship->GetID() == s)
			{
				roomShips.push_back(ship);
				break;
			}
		}
	}

	// Update all the ships in this room
	for (auto ship : roomShips)
	{
		ship->Update(timedelta);

		// Check collisions with ships
		if (ship == ships_.at(0))
		{
			checkCollisions(ship, roomShips);
		}
	}

	// Update my missile
	if (mymissile)
	{
		if (Ship* collision = (mymissile)->Update(roomShips, timedelta))
		{
			// Have collision
			collision->Injure(mymissile->GetDamage());
			delete mymissile;
			mymissile = nullptr;

			// Inform others that this ship was injured
			collision->SendObject(rakpeer_, ID_INJURED);

			std::cout << "COLLISION!" << std::endl;
		}
	}

	// Update network missiles in this room
	for (auto missileItr = missiles_.begin(); missileItr != missiles_.end(); ++missileItr)
	{
		Missile* missile = *missileItr;

		// Check if this missile is in this room
		bool missileIsInRoom = false;
		for (auto s : shipsInRoom)
		{
			if (missile->GetOwnerID() == s)
			{
				missileIsInRoom = true;
				break;
			}
		}

		// Do not care about missiles not in this room
		if (!missileIsInRoom)
		{
			continue;
		}

		// Collision checking with ships
		if (Ship* collision = missile->Update(roomShips, timedelta))
		{
			// Have collision
			delete missile;
			collision->Injure(missile->GetDamage());
			missiles_.erase(missileItr);

			std::cout << "COLLISION!" << std::endl;
			break;
		}
	}

	// Update the Goals
	for (auto goal : goalList)
	{
		goal->Update(timedelta);
	
		// Health Regen
		Ship* ship = ships_.at(0);
		if (
			ship->GetX() > goal->GetPosX() && ship->GetX() < goal->GetPosX() + goal->GetScaleX()
			&&
			ship->GetY() > goal->GetPosY() && ship->GetY() < goal->GetPosY() + goal->GetScaleY()
			)
		{
			ship->Heal(HEAL_RATE * timedelta);

			// Inform everyone that your health has increased
			RakNet::BitStream bs;

			bs.Write(static_cast<unsigned char>(ID_HEAL));
			bs.Write(ship->GetID());
			bs.Write(ship->GetHealth());

			rakpeer_->Send(&bs, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		}
	}

	// Kill/Kick the player
	if (ships_.at(0)->GetHealth() <= 0 && !leavingRoom)
	{
		leaveRoom();
	}

	// Handle the Packets that are received
	HandlePackets(rakpeer_->Receive());

	/*
	* Packets to Send
	*/
	if (RakNet::GetTime() - timer_ > NETWORK_UPDATE_DELTA)
	{
		timer_ = RakNet::GetTime();

		ships_.at(0)->SendObject(rakpeer_, ID_MOVEMENT);

		// Lab 13 Task 11 : send missile update 
		if (mymissile)
		{
			mymissile->SendObject(rakpeer_, ID_UPDATEMISSILE);
		}
	}

	return false;
}

void Application::joinRender()
{
	float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	// Renders the BG
	sprites_[ST_BG]->RenderEx(screenwidth * 0.5f, screenheight * 0.5f, 0);

	// Renders the Title
	font_->SetScale(2.5f);
	font_->printf(screenwidth * 0.5f, screenheight * 0.1f, HGETEXT_CENTER, "%s",
		"Enter your name:");

	// Show the user Input
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.5f, screenheight * 0.2f, HGETEXT_CENTER, "%s", inputBuffer[IBT_NORMAL].c_str());

	// Render error messages
	if (notifyMessage.length() > 0)
	{
		font_->SetColor(ARGB(255, 255, 0, 0));
		font_->SetScale(0.8f);
		font_->printf(screenwidth * 0.5f, screenheight * 0.9f, HGETEXT_CENTER, "%s", notifyMessage.c_str());
		font_->SetColor(ARGB(255, 255, 255, 255));
	}
}

void Application::lobbyRender()
{
	float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	// Renders the BG
	sprites_[ST_BG]->RenderEx(screenwidth * 0.5f, screenheight * 0.5f, 0);

	// Renders the Title
	font_->SetScale(2.5f);
	font_->printf(screenwidth * 0.5f, screenheight * 0.05f, HGETEXT_CENTER, "%s",
		"Game Lobby");

	// Renders the Buttons
	for (auto button : buttons)
	{
		button.Render();
	}

	// Renders the list of all players connected
	// Render the Title
	int shipNameYPos = screenheight * 0.2f;
	font_->SetColor(ARGB(255, 255, 255, 255));
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.8f, shipNameYPos, HGETEXT_RIGHT, "%s", "Player List");
	// Renders the list of players connected
	const int SHIP_Y_TITLE_OFFSET = 50;
	const int SHIP_EACH_Y_OFFSET = 30;
	shipNameYPos += SHIP_Y_TITLE_OFFSET;
	font_->SetScale(1.0f);
	for (auto ship : ships_)
	{
		font_->printf(screenwidth * 0.8f, shipNameYPos, HGETEXT_RIGHT, "%s", ship->GetName().c_str());
		shipNameYPos += SHIP_EACH_Y_OFFSET;
	}

	// Render the Room Title
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.2f, roomHeaderYPos, HGETEXT_LEFT, "%s", "Rooms List");
	// Renders the list of rooms
	for (auto bt : roomButtons)
	{
		bt.Render();
	}

	// Render error messages
	if (notifyMessage.length() > 0)
	{
		font_->SetColor(ARGB(255, 255, 0, 0));
		font_->SetScale(0.8f);
		font_->printf(screenwidth * 0.5f, screenheight * 0.8f, HGETEXT_CENTER, "%s", notifyMessage.c_str());
		font_->SetColor(ARGB(255, 255, 255, 255));
	}
}

bool Application::newRoomRender()
{
	float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	// Renders the BG
	sprites_[ST_BG]->RenderEx(screenwidth * 0.5f, screenheight * 0.5f, 0);

	// Renders the Title
	font_->SetScale(2.5f);
	font_->printf(screenwidth * 0.5f, screenheight * 0.1f, HGETEXT_CENTER, "%s",
		"Enter room name:");

	// Show the user Input
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.5f, screenheight * 0.2f, HGETEXT_CENTER, "%s", inputBuffer[IBT_NORMAL].c_str());

	return false;
}

void Application::gameRender()
{
	float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	auto shipsInRoom = currentRoom->GetConnectedIDs();
	for (auto ship : ships_)
	{
		// Check if this ship is in the room
		for (auto s : shipsInRoom)
		{
			if (ship->GetID() == s)
			{
				ship->Render();
				break;
			}
		}
	}

	// Render my missile
	if (mymissile)
	{
		mymissile->Render();
	}
	
	// Render other Missiles
	for (auto missile : missiles_)
	{
		// Check if this missile is in this room
		bool missileIsInRoom = false;
		for (auto s : shipsInRoom)
		{
			if (missile->GetOwnerID() == s)
			{
				missile->Render();
				break;
			}
		}
	}

	// Lab 13 Task 12 : Render network missiles
	for (auto missile : missiles_)
	{
		missile->Render();
	}

	// Render the Goals
	for (auto goal : goalList)
	{
		goal->Render();
	}

	// Render error messages
	if (notifyMessage.length() > 0)
	{
		font_->SetColor(ARGB(255, 255, 0, 0));
		font_->SetScale(0.8f);
		font_->printf(screenwidth * 0.5f, screenheight * 0.9f, HGETEXT_CENTER, "%s", notifyMessage.c_str());
		font_->SetColor(ARGB(255, 255, 255, 255));
	}
}

Ship * Application::findShip(int id)
{
	for (auto& ship : ships_)
	{
		if (ship->GetID() == id)
		{
			return ship;
		}
	}
}

void Application::createRoomButton(Room* rm)
{
	// Don't allow production of more buttons than there are rooms
	if (roomButtons.size() >= roomsList.size())
	{
		return;
	}

	float screenwidth = static_cast<float>(hge_->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge_->System_GetState(HGE_SCREENHEIGHT));

	RoomButton bt;

	bt.Init(rm, sprites_[ST_BUTTON], font_, screenwidth * 0.2f + 125, roomHeaderYPos + 10 + (roomButtons.size() + 1) * 55, 250, 50);
	roomButtons.push_back(bt);
}

void Application::joinRoom(int roomID)
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_JOINROOM;
	bs.Write(msgid);
	bs.Write(roomID);

	std::cout << "Attempting to join room #" << roomID << "!" << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	// Set the flag for joining a room
	joiningRoom = true;
}

void Application::leaveRoom(void)
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_LEAVEROOM;
	bs.Write(msgid);

	std::cout << "Attempting to leave room!" << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	// Set the flag for leaving a room
	leavingRoom = true;
}

Room * Application::findRoom(int roomID)
{
	for (auto& room : roomsList)
	{
		if (room->GetID() == roomID)
		{
			return room;
		}
	}

	return nullptr;
}

Room * Application::findRoomUserIsIn(int userID)
{
	for (auto& room : roomsList)
	{
		// Get the list of people connected to this room
		auto participants = room->GetConnectedIDs();
		// Find the user
		for (auto id : participants)
		{
			// We found the user
			if (userID == id)
			{
				// This is his room
				return room;
			}
		}
	}

	return nullptr;
}

/** 
* Kick starts the everything, called from main.
*/
void Application::Start()
{
	if (Init())
	{
		hge_->System_Start();
	}
}

bool Application::SendInitialPosition()
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_INITIALPOS;
	bs.Write(msgid);
	bs.Write(ships_.at(0)->GetName().c_str());
	bs.Write(ships_.at(0)->GetX());
	bs.Write(ships_.at(0)->GetY());

	std::cout << "Sending pos" << ships_.at(0)->GetX() << " " << ships_.at(0)->GetY() << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

bool Application::updateInputBuffer(int maxBufferLength, INPUT_BUFFER_TYPE type)
{
	static const double KEY_DELAY = 0.1f;
	static double timePassed = 0.0f;

	// Update the timer
	timePassed += hge_->Timer_GetDelta();

	if (hge_->Input_GetKeyState(HGEK_ENTER) && inputBuffer[type].size() > 0 && timePassed > KEY_DELAY)
	{
		timePassed = 0.0f;
		return true;
	}
	else if (hge_->Input_GetKeyState(HGEK_BACKSPACE) && timePassed > KEY_DELAY)
	{
		inputBuffer[type] = inputBuffer[type].substr(0, inputBuffer[type].length() - 1);
		timePassed = 0.0f;
	}
	else
	{
		char input = hge_->Input_GetKey();

		if (input >= ' ' && input <= '~' && inputBuffer[type].length() < maxBufferLength)
		{
			inputBuffer[type] += input;
		}
	}

	return false;
}

bool Application::checkCollisions(Ship* ship, ShipList shipsToCheckWith)
{
	for (auto& thisship = shipsToCheckWith.begin(); thisship != shipsToCheckWith.end(); thisship++)
	{
		if ((*thisship) == ship)
		{
			//skip if it is the same ship
			continue;
		}	

		if( ship->HasCollided( (*thisship) ) )
		{
			// Collision Check
			if( (*thisship)->CanCollide( RakNet::GetTime() ) &&  ship->CanCollide( RakNet::GetTime() ) )
			{
				std::cout << "collide!" << std::endl;

#ifdef INTERPOLATEMOVEMENT
			if( GetAbsoluteMag( ship->GetVelocityY() ) > GetAbsoluteMag( (*thisship)->GetVelocityY() ) )
			{
				// this transfers vel to thisship
				(*thisship)->SetVelocityY( (*thisship)->GetVelocityY() + ship->GetVelocityY()/3 );
				ship->SetVelocityY( - ship->GetVelocityY() );

				(*thisship)->SetServerVelocityY( (*thisship)->GetServerVelocityY() + ship->GetServerVelocityY()/3 );
				ship->SetServerVelocityY( - ship->GetServerVelocityY() );
			}
			else
			{
				ship->SetVelocityY( ship->GetVelocityY() + (*thisship)->GetVelocityY()/3 ); 
				(*thisship)->SetVelocityY( -(*thisship)->GetVelocityY()/2 );

				ship->SetServerVelocityY( ship->GetServerVelocityY() + (*thisship)->GetServerVelocityY()/3 ); 
				(*thisship)->SetServerVelocityY( -(*thisship)->GetServerVelocityY()/2 );
			}
			
			if( GetAbsoluteMag( ship->GetVelocityX() ) > GetAbsoluteMag( (*thisship)->GetVelocityX() ) )
			{
				// this transfers vel to thisship
				(*thisship)->SetVelocityX( (*thisship)->GetVelocityX() + ship->GetVelocityX()/3 );
				ship->SetVelocityX( - ship->GetVelocityX() );

				(*thisship)->SetServerVelocityX( (*thisship)->GetServerVelocityX() + ship->GetServerVelocityX()/3 );
				ship->SetServerVelocityX( - ship->GetServerVelocityX() );
			}
			else
			{
				// ship transfers vel to asteroid
				ship->SetVelocityX( ship->GetVelocityX() + (*thisship)->GetVelocityX()/3 ); 
				(*thisship)->SetVelocityX( -(*thisship)->GetVelocityX()/2 );

				ship->SetServerVelocityX( ship->GetServerVelocityX() + (*thisship)->GetServerVelocityX()/3 ); 
				(*thisship)->SetServerVelocityX( -(*thisship)->GetServerVelocityX()/2 );
			}

				ship->SetPreviousLocation();
#else
			if( GetAbsoluteMag( ship->GetVelocityY() ) > GetAbsoluteMag( (*thisship)->GetVelocityY() ) )
			{
				// this transfers vel to thisship
				(*thisship)->SetVelocityY( (*thisship)->GetVelocityY() + ship->GetVelocityY()/3 );
				ship->SetVelocityY( - ship->GetVelocityY() );
			}
			else
			{
				ship->SetVelocityY( ship->GetVelocityY() + (*thisship)->GetVelocityY()/3 ); 
				(*thisship)->SetVelocityY( -(*thisship)->GetVelocityY()/2 );
			}
			
			if( GetAbsoluteMag( ship->GetVelocityX() ) > GetAbsoluteMag( (*thisship)->GetVelocityX() ) )
			{
				// this transfers vel to thisship
				(*thisship)->SetVelocityX( (*thisship)->GetVelocityX() + ship->GetVelocityX()/3 );
				ship->SetVelocityX( - ship->GetVelocityX() );
			}
			else
			{
				// ship transfers vel to asteroid
				ship->SetVelocityX( ship->GetVelocityX() + (*thisship)->GetVelocityX()/3 ); 
				(*thisship)->SetVelocityX( -(*thisship)->GetVelocityX()/2 );
			}


//				ship->SetVelocityY( -ship->GetVelocityY() );
//				ship->SetVelocityX( -ship->GetVelocityX() );
			
				ship->SetPreviousLocation();
#endif
				// Send the collision
				(*thisship)->SendObject(rakpeer_, ID_COLLIDE);

				return true;
			}
				
		}

	}
	
	return false;
}

void Application::CreateMissile(float x, float y, float w, int id)
{
	// Lab 13 Task 9b : Implement networked version of createmissile
	RakNet::BitStream bs;
	unsigned char msgid;
	unsigned char deleted = 0;

	if (mymissile)
	{
		// Send update through network to delete this missile
		deleted = 1;
		msgid = ID_UPDATEMISSILE;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(deleted);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		// Delete existing missile
		delete mymissile;
		mymissile = nullptr;

		std::cout << "Deleted existing missile." << std::endl;
	}

	// Add a new missile based on the following parameter coordinates
	mymissile = new Missile("missile.png", x, y, w, id);
	// Send the network command to add new missile
	bs.Reset();
	msgid = ID_NEWMISSILE;
	bs.Write(msgid);
	bs.Write(id);
	bs.Write(x);
	bs.Write(y);
	bs.Write(w);
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	std::cout << "Created new missile." << std::endl;
}