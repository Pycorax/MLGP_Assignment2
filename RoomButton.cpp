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
	Update(0, 0, false);
}

void RoomButton::Update(int mousePosX, int mousePosY, bool leftClicked)
{
	// Update button status
	Button::Update(mousePosX, mousePosY, leftClicked);

	// Update the text
	ostringstream oss;
	oss << room->GetName() << " (" << room->GetConnectedIDs().size() << ")";
	buttonText = oss.str();
}

int RoomButton::GetRoomID(void) const
{
	return room->GetID();
}
