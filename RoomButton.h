#pragma once

// Inheritance Include
#include "Button.h"

// Other Includes
#include "MovementServer\Room.h"

class RoomButton : public Button
{
	Room* room;

public:
	RoomButton();
	~RoomButton();

	void Init(Room* rm, hgeSprite* sprite, hgeFont* font, int posX, int posY, int scaleX, int scaleY);
	void Update(int mousePosX, int mousePosY, bool leftClicked) override;

	int GetRoomID(void) const;
};

