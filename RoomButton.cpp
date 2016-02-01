#include "RoomButton.h"

// STL Includes
#include <sstream>
#include <exception>

// Using Directives
using std::ostringstream;
using std::runtime_error;

RoomButton::RoomButton()
{
}


RoomButton::~RoomButton()
{
}

void RoomButton::Init(Room* rm, hgeSprite * sprite, hgeFont * font, int posX, int posY, int scaleX, int scaleY)
{
	if (rm == nullptr)
	{
		throw new runtime_error("Invalid room passed in!");
	}

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
	oss << room->GetName() << " (" << room->GetTeamList(Room::TEAM_BLUE).size() << " - " << room->GetTeamList(Room::TEAM_ORANGE).size() << ")";
	buttonText = oss.str();
}

int RoomButton::GetRoomID(void) const
{
	return room->GetID();
}
