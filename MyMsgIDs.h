#ifndef MYSMSGIDS_H_
#define MYSMSGIDS_H_

#include "MessageIdentifiers.h"

enum MyMsgIDs
{
	// Connection
	ID_WELCOME = ID_USER_PACKET_ENUM,
	ID_SERVER_FULL,
	// Rooms
	ID_NEWROOM,
	ID_JOINROOM,
	ID_LEAVEROOM,
	// Ships
	ID_NEWSHIP,
	ID_LOSTSHIP,
	ID_INITIALPOS,
	ID_MOVEMENT,
	ID_COLLIDE,
	ID_INJURED,
	// Missiles
	ID_NEWMISSILE,
	ID_UPDATEMISSILE,
	// Ball
	ID_UPDATEGOAL
};

#endif