#include "Goal.h"



Goal::Goal() : INetworkProcessable()
	, m_goalID(0)
	, m_posX(0)
	, m_posY(0)
	, m_startPosY(0)
	, m_endPosY(0)
	, m_speed(0.0f)
	, m_scaleX(1)
	, m_scaleY(0)
	, m_forward(true)
{
}


Goal::~Goal()
{
}

void Goal::Init(int id, float startPosX, float startPosY, float endPosY, float speed, float scaleX, float scaleY)
{
	m_goalID = id;
	m_posX = startPosX;

	// Ensure that startPos is always smaller than endPos
	if (startPosY > endPosY)
	{
		m_endPosY = startPosY;
		m_startPosY = endPosY;
	}
	else
	{
		m_startPosY = startPosY;
		m_endPosY = endPosY;
	}

	m_posY = m_startPosY;
	m_scaleX = scaleX;
	m_scaleY = scaleY;
}

void Goal::Update(double dt)
{
	if (m_forward)
	{
		m_posY += m_speed * dt;

		if (m_posY > m_endPosY)
		{
			m_posY = m_endPosY;
			m_forward = false;
		}
	}
	else
	{
		m_posY += m_speed * dt;

		if (m_posY < m_startPosY)
		{
			m_posY = m_startPosY;
			m_forward = false;
		}
	}
}

float Goal::GetPosX(void) const
{
	return m_posX;
}

float Goal::GetPosY(void) const
{
	return m_posY;
}

float Goal::GetScaleX(void) const
{
	return m_scaleX;
}

float Goal::GetScaleY(void) const
{
	return m_scaleY;
}

void Goal::SendObject(RakPeerInterface * peer, MyMsgIDs type) const
{
	RakNet::BitStream bs;

	// Note down the type of message
	bs.Write(static_cast<unsigned char>(type));

	switch (type)
	{
		case ID_WELCOME:
		{
			bs.Write(m_goalID);
			bs.Write(m_posX);
			bs.Write(m_posY);
			bs.Write(m_startPosY);
			bs.Write(m_endPosY);
			bs.Write(m_scaleX);
			bs.Write(m_scaleY);
			bs.Write(m_speed);
			bs.Write(m_forward);
			break;
		}

		case ID_UPDATEGOAL:
		{
			bs.Write(m_goalID);
			bs.Write(m_posX);
			bs.Write(m_posY);
			bs.Write(m_scaleX);
			bs.Write(m_scaleY);
			bs.Write(m_speed);
			bs.Write(m_forward);
		}
		break;
	}

	// Broadcast to everyone
	peer->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Goal::SendObject(RakNet::BitStream * bs, MyMsgIDs type) const
{
	switch (type)
	{
		case ID_WELCOME:
		{
			bs->Write(m_goalID);
			bs->Write(m_posX);
			bs->Write(m_posY);
			bs->Write(m_startPosY);
			bs->Write(m_endPosY);
			bs->Write(m_scaleX);
			bs->Write(m_scaleY);
			bs->Write(m_speed);
			bs->Write(m_forward);
			break;
		}
	}
}

void Goal::RecvObject(RakNet::BitStream * bs, MyMsgIDs type)
{
	switch (type)
	{
		case ID_WELCOME:
		{
			bs->Read(m_goalID);
			bs->Read(m_posX);
			bs->Read(m_posY);
			bs->Read(m_startPosY);
			bs->Read(m_endPosY);
			bs->Read(m_scaleX);
			bs->Read(m_scaleY);
			bs->Read(m_speed);
			bs->Read(m_forward);
			break;
		}

		case ID_UPDATEGOAL:
		{
			int goalID = -1;
			bs->Read(goalID);

			// If we are the correct recipient
			if (goalID == m_goalID)
			{
				// Update ourselves with these values
				bs->Read(m_posX);
				bs->Read(m_posY);
				bs->Read(m_scaleX);
				bs->Read(m_scaleY);
				bs->Read(m_speed);
				bs->Read(m_forward);
			}
			else
			{
				// Reset this, let someone else handle it
				bs->ResetReadPointer();
			}

			break;
		}
	}
}
