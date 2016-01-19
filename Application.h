#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "ship.h"
#include "missile.h"
#include <vector>

class HGE;
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
	/*
	 * Application
	 */
	HGE* hge_; //!< Instance of the internal graphics engine
	
	/*
	 * Network
	 */
	RakPeerInterface* rakpeer_;
	unsigned int timer_;
	const int TICK_RATE = 24;
	const int NETWORK_UPDATE_DELTA = 1000 / TICK_RATE;
	
	// Lab 13 Task 1 : add variables for local missle

	/*
	 * Gameplay
	 */
	// Ships
	typedef std::vector<Ship*> ShipList;  //!< A list of ships
	ShipList ships_; //!< List of all the ships in the universe
	// Missiles
	Missile* mymissile;
	bool have_missile;
	bool keydown_enter;
	typedef std::vector<Missile*> MissileList;
	MissileList missiles_;

	// App Lifetime
	bool Init();
	static bool Loop();
	void Shutdown();
	bool ControlUpdate(double dt);
	bool HandlePackets(Packet* packet);

	// Others
	bool checkCollisions(Ship* ship);
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