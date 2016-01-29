#include "Button.h"

#include <hgefont.h>
#include <hgesprite.h>

Button::Button()
	: positionX(0)
	, positionY(0)
	, scaleX(0)
	, scaleY(0)
	, buttonText("")
	, pressed(false)
{
}


Button::~Button()
{
}

void Button::Init(hgeSprite * sprite, hgeFont * font, int posX, int posY, int scaleX, int scaleY, string text)
{
	this->sprite = sprite;
	this->font = font;

	positionX = posX;
	positionY = posY;
	this->scaleX = scaleX;
	this->scaleY = scaleY;

	buttonText = text;
}

void Button::Update(int mousePosX, int mousePosY, bool leftClicked)
{
	if (leftClicked
		&&
		mousePosX > positionX - scaleX * 0.5f && mousePosX < positionX + scaleX * 0.5f
		&&
		mousePosY > positionY - scaleY * 0.5f && mousePosY < positionY + scaleY * 0.5f
		)
	{
		pressed = true;
	}
	else
	{
		pressed = false;
	}
}

void Button::Render(void) const
{
	// Render the button
	sprite->RenderStretch(positionX - scaleX * 0.5f, positionY - scaleY * 0.5f, positionX + scaleX * 0.5f, positionY + scaleY * 0.5f);
	// Render the button text
	font->SetScale(1.0f);
	font->SetColor(ARGB(255, 0, 0, 0));
;	font->printf(positionX, positionY - 10.0f, HGETEXT_CENTER, "%s", buttonText.c_str());
	font->SetColor(ARGB(255, 255, 255, 255));
}

bool Button::GetState(void) const
{
	return pressed;
}

