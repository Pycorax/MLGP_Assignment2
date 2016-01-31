#pragma once

// Other Includes
#include "../INetworkProcessable.h"

/*
 * A simple goal that moves from top to bottom
 */
class Goal : public INetworkProcessable
{
protected:
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
	Goal();
	virtual ~Goal();

	// Lifetime
	void Init(int id, float startPosX, float startPosY, float endPosY, float speed, float scaleX, float scaleY);
	void Update(double dt);

	// Getters
	int GetID(void) const;
	float GetPosX(void) const;
	float GetPosY(void) const;
	float GetScaleX(void) const;
	float GetScaleY(void) const;

	// INetworkProcessable
	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const;
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

