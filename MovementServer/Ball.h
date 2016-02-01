#pragma once

// Inheritance Includes
#include "../INetworkProcessable.h"

class Ball : public INetworkProcessable
{
	// Positioning
	float x_; //!< The x-cordinate of the ship
	float y_; //!< The y-cordinate of the ship
	float w_; //!< The angular position of the ship

			  // Movement
	float velocity_x_; //!< The resolved velocity of the ship along the x-axis
	float velocity_y_; //!< The resolved velocity of the ship along the y-axis

	float oldx, oldy;	// for reset back to previous location if collision detected

	float server_x_;
	float server_y_;
	float server_w_;
	float client_x_;
	float client_y_;
	float client_w_;
	float server_velx_;
	float server_vely_;
	float ratio_;

	unsigned int id;
	float angular_velocity;

	unsigned int collidetimer;
public:
	static const int MAX_NAME_LENGTH = 15;

	Ball(float locx_ = 0.0f, float locy_ = 0.0f);
	~Ball();
	void Update(float timedelta);
	void Render();
	void Accelerate(float acceleration, float timedelta);

	bool HasCollided(Ship *ship);

	float GetVelocityX() { return velocity_x_; }
	float GetVelocityY() { return velocity_y_; }

	void SetVelocityX(float velocity) { velocity_x_ = velocity; }
	void SetVelocityY(float velocity) { velocity_y_ = velocity; }

	float GetAngularVelocity() { return angular_velocity; }

	void SetAngularVelocity(float av) { angular_velocity = av; }

	void SetPreviousLocation()
	{
		x_ = oldx;
		y_ = oldy;
	}

	unsigned int GetID() { return id; }

	void setID(unsigned int newid) { id = newid; }

	void setLocation(float x, float y, float w)
	{
		x_ = x;
		y_ = y;
		w_ = w;
	}

	void SetX(float x) { x_ = x; }
	void SetY(float y) { y_ = y; }

	float GetX() { return x_; }
	float GetY() { return y_; }
	float GetW() { return w_; }

	bool CanCollide(unsigned int timer)
	{
		if (timer - collidetimer > 2000)
		{
			collidetimer = timer;

			return true;
		}

		return false;
	}

	void SetServerLocation(float x, float y, float w) {
		server_x_ = x;
		server_y_ = y;
		server_w_ = w;
	}

	void SetServerVelocityX(float velocity) { server_velx_ = velocity; }
	void SetServerVelocityY(float velocity) { server_vely_ = velocity; }

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


	/*
	* INetworkProccessable
	*/
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const;
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

#endif