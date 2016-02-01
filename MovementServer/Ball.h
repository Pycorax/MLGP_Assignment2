#pragma once

// Inheritance Includes
#include "../INetworkProcessable.h"

class Ball : public INetworkProcessable
{
protected:
	// Positioning
	float x_; //!< The x-cordinate of the ship
	float y_; //!< The y-cordinate of the ship
	float w_; //!< The angular position of the ship

	// Movement
	float velocity_x_; //!< The resolved velocity of the ship along the x-axis
	float velocity_y_; //!< The resolved velocity of the ship along the y-axis
	float angular_velocity;

	// Collision
	float oldx, oldy;	// for reset back to previous location if collision detected
	unsigned int collidetimer;

public:
	Ball(float locx_ = 0.0f, float locy_ = 0.0f)
		: w_(0)
		, angular_velocity(0)
		, velocity_x_(0)
		, velocity_y_(0)
		, collidetimer(0)
	{
		x_ = locx_;
		y_ = locy_;
	}

	~Ball()
	{

	}
	virtual void Update(float timedelta)
	{
		float pi = 3.141592654f * 2;

		if (w_ > pi)
			w_ -= pi;

		if (w_ < 0.0f)
			w_ += pi;

		// store old coords
		oldx = x_;
		oldy = y_;
	}

	virtual void Accelerate(float acceleration, float timedelta)
	{
		velocity_x_ += acceleration * cosf(w_) * timedelta;
		velocity_y_ += acceleration * sinf(w_) * timedelta;
	}

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

	/*
	* INetworkProccessable
	*/
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const
	{
		
	}
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const
	{
		switch (type)
		{
			case ID_UPDATEBALL:
			{
				bs->Write(x_);
				bs->Write(y_);
				bs->Write(w_);

				bs->Write(velocity_x_);
				bs->Write(velocity_y_);
				bs->Write(angular_velocity);
			}
			break;
		}
	}
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type)
	{
		switch (type)
		{
			case ID_COLLIDEBALL:
			{
				bs->Read(x_);
				bs->Read(y_);
				bs->Read(velocity_x_);
				bs->Read(velocity_y_);
			}
			break;
		}
	}
};