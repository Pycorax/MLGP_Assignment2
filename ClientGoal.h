#pragma once

// Inheritance Include
#include "INetworkProcessable.h"

// API Include
#include <hge.h>
#include <hgerect.h>

class hgeSprite;

class ClientGoal : public INetworkProcessable
{

	// HGE
	HTEXTURE tex_; //!< Handle to the sprite's texture
	hgeSprite* sprite_; //!< The sprite used to display the ship
	hgeRect collidebox;

	// ID used to identify the goal
	int m_goalID;

	// Stores Current Position
	float m_posX;
	float m_posY;
	// Stores the waypoints for moving
	float m_startPosY;
	float m_endPosY;
	// Stores the speed for moving
	float m_speed;
	// Stores the scale
	float m_scaleX;
	float m_scaleY;

	// Stores the current direction
	bool m_forward;


public:
	ClientGoal();
	~ClientGoal();

	void Init(hgeSprite* sprite, int id, float startPosX, float startPosY, float endPosY, float speed, float scaleX, float scaleY);
	void Render();
	void Update(double dt);

	// Setters
	void SetSprite(hgeSprite* sprite);

	// Getters
	float GetPosX(void) const;
	float GetPosY(void) const;
	float GetScaleX(void) const;
	float GetScaleY(void) const;

	// INetworkProcessable
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const;
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

