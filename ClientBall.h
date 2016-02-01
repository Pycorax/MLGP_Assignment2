#pragma once

// Inheritance Includes
#include "MovementServer\Ball.h"

// Other Includes
#include "ship.h"

// Forward Declarations
class hgeSprite;

class ClientBall : public Ball
{
	// Rendering
	hgeSprite* sprite_;

	// Collision
	hgeRect collidebox;

	// Interpolation
	float server_x_;
	float server_y_;
	float server_w_;
	float client_x_;
	float client_y_;
	float client_w_;
	float server_velx_;
	float server_vely_;
	float ratio_;

public:
	ClientBall(hgeSprite* sprite = nullptr);
	~ClientBall();

	void Update(float timedelta);
	void Render();

	// Collision
	hgeRect* GetBoundingBox();
	bool HasCollided(Ship *ship);
	
	bool CanCollide(unsigned int timer)
	{
		if (timer - collidetimer > 2000)
		{
			collidetimer = timer;

			return true;
		}

		return false;
	}

	// Interpolation
	void SetServerLocation(float x, float y, float w) 
	{
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

	// INetworkProcessable
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type);
	virtual void SendObject(RakNet::BitStream * bs, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

