#ifndef _SHIP_H_
#define _SHIP_H_

#include <hge.h>
#include <hgerect.h>
#include <memory>
#include <string>
#include "INetworkProcessable.h"

class hgeSprite;
class hgeFont;

#define INTERPOLATEMOVEMENT 

/**
* The Ship class represents a single spaceship floating in space. It obeys
* 2D physics in terms of displacement, velocity and acceleration, as well
* as angular position and displacement. The size of the current art is
* 128*128 pixels
*/

class Ship : public INetworkProcessable
{
	// HGE
	HTEXTURE tex_; //!< Handle to the sprite's texture
	hgeSprite* sprite_; //!< The sprite used to display the ship
	hgeSprite* boomSprite_;
	std::auto_ptr<hgeFont> font_;
	hgeRect collidebox;

	// Ship Name
	std::string mytext_;

	// Ship Health
	static const int MAX_HEALTH = 100;
	static const int MIN_HEALTH = 0;
	int health_;

	// Boom
	static const double BOOM_SHOW_TIME;
	double boomShowTimer;

	// Positioning
	float x_; //!< The x-cordinate of the ship
	float y_; //!< The y-cordinate of the ship
	float w_; //!< The angular position of the ship

	// Movement
	float velocity_x_; //!< The resolved velocity of the ship along the x-axis
	float velocity_y_; //!< The resolved velocity of the ship along the y-axis

	float oldx, oldy;	// for reset back to previous location if collision detected

	// Lab Task 2 : add for interpolation
#ifdef INTERPOLATEMOVEMENT
	float server_x_;
	float server_y_;
	float server_w_;
	float client_x_;
	float client_y_;
	float client_w_;
	float server_velx_;
	float server_vely_;
	float ratio_;
#endif

	unsigned int id;
	float angular_velocity;

	unsigned int collidetimer;
public:
	static const int MAX_NAME_LENGTH = 15;

	Ship(float locx_, float locy_);
	~Ship();
	void Update(float timedelta);
	void Render();
	void Accelerate(float acceleration, float timedelta);

	void SetName(const char * text);
	
	void SetSprite(hgeSprite* sprite) { sprite_ = sprite; }
	void SetBoomSprite(hgeSprite* sprite) { boomSprite_ = sprite; }

	hgeRect* GetBoundingBox();
	bool HasCollided( Ship *ship );

	float GetVelocityX() { return velocity_x_; }
	float GetVelocityY() { return velocity_y_; }

	void SetVelocityX( float velocity ) { velocity_x_ = velocity; }
	void SetVelocityY( float velocity ) { velocity_y_ = velocity; }

	float GetAngularVelocity() { return angular_velocity; }

	void SetAngularVelocity( float av ) { angular_velocity = av; }

	void SetPreviousLocation()
	{
		x_ = oldx;
		y_ = oldy;
	}

	unsigned int GetID() { return id; }

	void setID(unsigned int newid ) { id = newid; }

	void setLocation( float x, float y, float w ) 
	{ 
		x_ = x; 
		y_ = y; 
		w_ = w; 
	}

	void SetX( float x ) { x_ = x; }
	void SetY( float y ) { y_ = y; }

	float GetX() { return x_; }
	float GetY() { return y_; }
	float GetW() { return w_; }

	std::string GetName() { return mytext_; }

	bool CanCollide( unsigned int timer ) 
	{
		if( timer - collidetimer > 2000 )
		{
			collidetimer = timer;

			return true;
		}

		return false;
	}

	// Lab Task 2 : add new member functions here
#ifdef INTERPOLATEMOVEMENT
	void SetServerLocation( float x, float y, float w ) { 
		server_x_ = x; 
		server_y_ = y;
		server_w_ = w;
	}

	void SetServerVelocityX( float velocity ) { server_velx_ = velocity; }
	void SetServerVelocityY( float velocity ) { server_vely_ = velocity; }

	float GetServerVelocityX() { return server_velx_; }
	float GetServerVelocityY() { return server_vely_; }

	float GetServerX() { return server_x_; }
	float GetServerY() { return server_y_; }
	float GetServerW() { return server_w_; }

	void DoInterpolateUpdate()
	{
		client_x_ = x_;
		client_y_ = y_;
		client_w_ = w_;
		velocity_x_ = server_velx_;
		velocity_y_ = server_vely_;
		ratio_ = 0;
	}

	// Health
	bool IsAlive(void) { return health_ > MIN_HEALTH; }
	int GetHealth(void) { return health_; }
	void Injure(int damage);
	void Heal(int health);
	void ResetHealth(void);
#endif

	/*
	* INetworkProccessable
	*/
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const;
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

#endif