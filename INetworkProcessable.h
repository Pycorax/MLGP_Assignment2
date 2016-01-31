#pragma once

#include "MyMsgIDs.h"
#include "RakPeerInterface.h"
#include "BitStream.h"

class INetworkProcessable
{
public:
	INetworkProcessable() {}
	virtual ~INetworkProcessable() {}

	virtual void SendObject(RakPeerInterface* peer, MyMsgIDs type) const = 0;
	virtual void SendObject(RakNet::BitStream* bs, MyMsgIDs type) const = 0;
	virtual void RecvObject(RakNet::BitStream* bs, MyMsgIDs type) = 0;
};
