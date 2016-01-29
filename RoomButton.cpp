#include "RoomButton.h"

// STL Includes
#include <sstream>

// Using Directives
using std::ostringstream;

RoomButton::RoomButton()
{
}


RoomButton::~RoomButton()
{
}

void RoomButton::Init(Room* rm, hgeSprite * sprite, hgeFont * font, int posX, int posY, int scaleX, int scaleY)
{
	room = rm;
	Button::Init(sprite, font, posX, posY, scaleX, scaleY, "");
	UpdateText();
}

void RoomButton::UpdateText(void)
{
	ostringstream oss;
	oss << room->GetName() << " (" << room->GetConnectedIDs().size() << ")";
	buttonText = oss.str();
}

int RoomButton::GetRoomID(void) const
{
	return room->GetID();
}
