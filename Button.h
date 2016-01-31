#pragma once

// Includes
#include <memory>
#include <hge.h>
#include <hgerect.h>
#include <string>

// Forward Declarations
class hgeSprite;
class hgeFont;

// Using Directives
using std::string;

class Button
{
protected:
	// Position and Scale
	int positionX;
	int positionY;
	int scaleX;
	int scaleY;

	// HGE
	HTEXTURE tex_; //!< Handle to the sprite's texture
	hgeSprite* sprite; //!< The sprite used to display the ship
	hgeFont* font;
	hgeRect collidebox;

	// Text
	string buttonText;

	// State
	bool pressed;

public:
	Button();
	~Button();

	void Init(hgeSprite* sprite, hgeFont* font, int posX, int posY, int scaleX, int scaleY, string text = "");
	virtual void Update(int mousePosX, int mousePosY, bool leftClicked);
	void Render(void) const;

	bool GetState(void) const;
};

