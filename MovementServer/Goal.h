#pragma once

// Other Includes
#include "../INetworkProcessable.h"

/*
 * A simple goal that moves from top to bottom
 */
class Goal : public INetworkProcessable
{
	// ID used to identify the goal
	int m_goalID;

	// Stores Current Position
	int m_posX;
	int m_posY;
	// Stores the waypoints for moving
	int m_startPosY;
	int m_endPosY;
	// Stores the speed for moving
	float m_speed;
	// Stores the scale
	int m_scaleX;
	int m_scaleY;

	// Stores the current direction
	bool m_forward;

public:
	Goal();
	virtual ~Goal();

	void Init(int id, int startPosX, int startPosY, int endPosY, float speed, int scaleX, int scaleY);
	void Update(double dt);

	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type);
};

