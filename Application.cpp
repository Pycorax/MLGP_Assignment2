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

/** 
* Constuctor
*
* Creates an instance of the graphics engine and network engine
*/

Application::Application()
	: hge_(hgeCreate(HGE_VERSION))
	, appstate(AS_JOIN)
	, rakpeer_(RakNetworkFactory::GetRakPeerInterface())
	, timer_(0)
	// Lab 13 Task 2 : add new initializations
	, mymissile(nullptr)
	, keydown_enter(false)
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

		// Load Sprites
		sprites_[ST_BG] = new hgeSprite(textures_[TT_BG], 0, 0, screenwidth, screenheight);
		sprites_[ST_BG]->SetHotSpot(screenwidth * 0.5f, screenheight * 0.5f);
		sprites_[ST_BOOM] = new hgeSprite(textures_[TT_BOOM], 0, 0, 40, 40);
		sprites_[ST_BOOM]->SetHotSpot(20, 20);

		// Load Fonts
		font_ = new hgeFont("font1.fnt");
		font_->SetScale(0.5);

		// Set up the Buttons
		buttons[BT_START].Init(sprites_[ST_BOOM], font_, 30, 30, 60, 30, "Test");

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
	for (size_t i = 0; i < TT_TOTAL; ++i)
	{
		hge_->Texture_Free(textures_[i]);
	}

	for (size_t i = 0; i < ST_TOTAL; ++i)
	{
		delete sprites_[i];
		sprites_[i] = nullptr;
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
	if (hge_->Input_GetKeyState(HGEK_ENTER))
	{
		if (!keydown_enter)
		{
			CreateMissile(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID());
		}
	}
	else
	{
		if (keydown_enter)
		{
			keydown_enter = false;
		}
	}	

	return false;
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
			}

			break;

		case ID_WELCOME:
		{
			unsigned int shipcount, id;
			float x_, y_;
			int type_;
			char chartemp[5];
			char nameArray[Ship::MAX_NAME_LENGTH];

			bs.Read(id);
			ships_.at(0)->setID(id);
			bs.Read(shipcount);
			
			// Receive list of all ships
			for (unsigned int i = 0; i < shipcount; ++i)
			{
				bs.Read(nameArray);
				bs.Read(id);
				bs.Read(x_);
				bs.Read(y_);
				bs.Read(type_);
				std::cout << "New Ship " << nameArray << " at pos" << x_ << " " << y_ << std::endl;
				Ship* ship = new Ship(type_, x_, y_);
				ship->SetName(nameArray);
				ship->setID(id);
				ships_.push_back(ship);
			}

			
			// Receive rooms data
			// - Define variables for reading
			char roomName[Room::MAX_ROOM_NAME_LENGTH];
			int numShips = 0;
			int shipID = 0;
			// -- Get number of rooms
			unsigned numRooms = 0;
			bs.Read(numRooms);

			// -- Iterate through the rooms we expect
			for (unsigned int i = 0; i < numRooms; ++i)
			{
				// Initialize each room
				// -- Get the room name
				bs.Read(roomName);

				// -- Create the room
				roomsList.push_back(Room(roomName));

				// -- Get the number of ships in this room
				bs.Read(numShips);

				// -- Receive the IDs of ships in this room
				for (unsigned int ship = 0; ship < numShips; ++ship)
				{
					// Retrieve the ship ID
					bs.Read(shipID);

					// Add the ID in
					roomsList.back().AddUser(shipID);
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
			roomsList.push_back(Room(roomName, roomID));

			std::cout << "New Room Created on Server: " << roomName << " #" << roomID << std::endl;
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
				int type_;
				char name_[Ship::MAX_NAME_LENGTH];

				bs.Read(name_);
				bs.Read(x_);
				bs.Read(y_);
				bs.Read(type_);
				std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
				Ship* ship = new Ship(type_, x_, y_);
				ship->SetName(name_);
				ship->setID(id);
				ships_.push_back(ship);
			}

		}
		break;

		case ID_LOSTSHIP:
		{
			unsigned int shipid;
			bs.Read(shipid);
			for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
			{
				if ((*itr)->GetID() == shipid)
				{
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
				float x, y;
				bs.Read(shipid);

				if (shipid == ships_.at(0)->GetID())
				{
					std::cout << "hit by someone!" << std::endl;

					ships_.at(0)->RecvObject(&bs, ID_INJURED);
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
	static bool connected = false;

	if (updateInputBuffer(Ship::MAX_NAME_LENGTH))
	{
		// Reset the input buffer
		inputBuffer = "";

		// Connect to the server
		if (!connected)
		{
			if (rakpeer_->Connect("127.0.0.1", 1691, 0, 0))
			{
				// Initialize ships
				ships_.push_back(new Ship(rand() % 4 + 1, rand() % 500 + 100, rand() % 400 + 100));
				ships_.at(0)->SetName(inputBuffer.c_str());
				connected = true;
			}
			else
			{
				// Inform the user that we can't connect
				notifyMessage = "Unable to connect to server!";
			}
		}
	}

	// If a Welcome Message was received, then go to the lobby
	int msgID = HandlePackets(rakpeer_->Receive());
	switch (msgID)
	{
		case ID_WELCOME:
			// Go to the next state
			appstate = AS_LOBBY;
			break;
		case ID_SERVER_FULL:
			// Inform the user of the issue
			notifyMessage = "Server is FULL!";
			connected = false;
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			connected = false;
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

	if (buttons[BT_START].GetState())
	{
		appstate = AS_GAME;
	}

	// Create new room
	if (hge_->Input_GetKeyState(HGEK_R))
	{
		appstate = AS_NEWROOM;
	}

	// Packet Handling
	HandlePackets(rakpeer_->Receive());

	return false;
}

bool Application::newRoomUpdate()
{
	// Once the user has finished typing,
	if (updateInputBuffer(Room::MAX_ROOM_NAME_LENGTH))
	{
		// Tell the server to create this room
		// -- Create BitStream object
		RakNet::BitStream bs;

		// State the message type
		bs.Write(static_cast<unsigned char>(ID_NEWROOM));

		// Ship Update
		bs.Write(inputBuffer.c_str());

		// Send this message
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		// Clear the input buffer
		inputBuffer = "";

		// Go back to the lobby
		appstate = AS_LOBBY;
	}

	// If cancel creating new room
	if (hge_->Input_GetKeyState(HGEK_ESCAPE))
	{
		// Reset the input buffer
		inputBuffer = "";

		// Go back to the lobby
		appstate = AS_LOBBY;
	}

	return false;
}

bool Application::gameUpdate()
{
	// Get the delta time
	float timedelta = hge_->Timer_GetDelta();

	// Reset my angular velocity
	ships_.at(0)->SetAngularVelocity(0.0f);

	// Get Controls
	if (ControlUpdate(timedelta))
	{
		// True is returned to end the game
		return true;
	}

	// Update all the ships
	for (ShipList::iterator ship = ships_.begin(); ship != ships_.end(); ship++)
	{
		(*ship)->Update(timedelta);

		//collisions
		if ((*ship) == ships_.at(0))
			checkCollisions((*ship));
	}

	// Update my missile
	if (mymissile)
	{
		if (Ship* collision = (mymissile)->Update(ships_, timedelta))
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

	// Update network missiles
	for (auto missile = missiles_.begin(); missile != missiles_.end(); ++missile)
	{
		if (Ship* collision = (*missile)->Update(ships_, timedelta))
		{
			// Have collision
			delete *missile;
			collision->Injure((*missile)->GetDamage());
			missiles_.erase(missile);

			std::cout << "COLLISION!" << std::endl;
			break;
		}
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
	font_->printf(screenwidth * 0.5f, screenheight * 0.2f, HGETEXT_CENTER, "%s", inputBuffer.c_str());

	// Render error messages
	if (notifyMessage.length() > 0)
	{
		font_->SetColor(ARGB(255, 255, 0, 0));
		font_->SetScale(0.8f);
		font_->printf(screenwidth * 0.5f, screenheight * 0.9f, HGETEXT_CENTER, "Error: %s", notifyMessage.c_str());
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
	font_->printf(screenwidth * 0.5f, screenheight * 0.1f, HGETEXT_CENTER, "%s",
		"Game Lobby");

	// Renders the Buttons
	for (auto button : buttons)
	{
		button.Render();
	}

	// Renders the list of all players connected
	// Render the Room Title
	int shipNameYPos = screenheight * 0.3f;
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.8f, shipNameYPos, HGETEXT_RIGHT, "%s",
		"Player List");
	// Renders the list of rooms
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
	int roomYPos = screenheight * 0.3f;
	font_->SetScale(1.5f);
	font_->printf(screenwidth * 0.2f, roomYPos, HGETEXT_LEFT, "%s",
		"Rooms List");
	// Renders the list of rooms
	const int ROOM_Y_TITLE_OFFSET = 50;
	const int ROOM_EACH_Y_OFFSET = 30;
	roomYPos += ROOM_Y_TITLE_OFFSET;
	font_->SetScale(1.0f);
	for (auto room : roomsList)
	{
		// Render the name of a room
		font_->printf(screenwidth * 0.2f, roomYPos, HGETEXT_LEFT, "%s",
			room.GetName().c_str());

		roomYPos += ROOM_EACH_Y_OFFSET;
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
	font_->printf(screenwidth * 0.5f, screenheight * 0.2f, HGETEXT_CENTER, "%s", inputBuffer.c_str());

	return false;
}

void Application::gameRender()
{
	ShipList::iterator itr;
	for (itr = ships_.begin(); itr != ships_.end(); itr++)
	{
		(*itr)->Render();
	}

	// Lab 13 Task 6 : Render the missile
	if (mymissile)
	{
		mymissile->Render();
	}

	// Lab 13 Task 12 : Render network missiles
	for (auto missile : missiles_)
	{
		missile->Render();
	}
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
	bs.Write(ships_.at(0)->GetType());

	std::cout << "Sending pos" << ships_.at(0)->GetX() << " " << ships_.at(0)->GetY() << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

bool Application::updateInputBuffer(int maxBufferLength)
{
	static const double BACKSPACE_DELAY = 0.1f;
	static double timePassed = 0.0f;

	// Update the timer
	timePassed += hge_->Timer_GetDelta();

	if (hge_->Input_GetKeyState(HGEK_ENTER))
	{
		return true;
	}
	else if (hge_->Input_GetKeyState(HGEK_BACKSPACE) && timePassed > BACKSPACE_DELAY)
	{
		inputBuffer = inputBuffer.substr(0, inputBuffer.length() - 1);
		timePassed = 0.0f;
	}
	else
	{
		char input = hge_->Input_GetKey();

		if (input >= ' ' && input <= '~' && inputBuffer.length() < maxBufferLength)
		{
			inputBuffer += input;
		}
	}

	return false;
}

bool Application::checkCollisions(Ship* ship)
{
	for (std::vector<Ship*>::iterator thisship = ships_.begin();
		thisship != ships_.end(); thisship++)
	{
		if( (*thisship) == ship ) continue;	//skip if it is the same ship

		if( ship->HasCollided( (*thisship) ) )
		{
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
}