#include "ColorSystem.h"

#include <iostream>
#include <vector>

using std::vector;
using std::cout;

Color::COLOR_TYPE Color::GetColorTypeFromSelector(char ch)
{
	for (size_t i = 0; i < E_MAX_COLORS; ++i)
	{
		if (ch == COLOR_SELECTOR[i])
		{
			return static_cast<COLOR_TYPE>(i);
		}
	}

	return E_MAX_COLORS;
}

void Color::SetConsoleTextColor(WORD attrib)
{
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hstdout, attrib);
}

void Color::SetConsoleTextColor(COLOR_TYPE color)
{
	WORD finalColour;

	//Base colours
	if (color >= E_RED_COLOR && color <= E_MAGENTA_COLOR) //Red Base
	{
		finalColour = FOREGROUND_RED;

		if (color == E_YELLOW_COLOR)
		{
			finalColour |= FOREGROUND_GREEN;
			finalColour |= FOREGROUND_INTENSITY;
		}

		if (color == E_PURPLE_COLOR || color == E_MAGENTA_COLOR)
		{
			finalColour |= FOREGROUND_BLUE;
		}

		//Intensity
		if (color >= E_BRIGHT_RED_COLOR && color <= E_MAGENTA_COLOR)
		{
			finalColour |= FOREGROUND_INTENSITY;
		}
	}
	else if (color >= E_BLUE_COLOR && color <= E_LIGHT_BLUE_COLOR) //Blue Base
	{
		finalColour = FOREGROUND_BLUE;

		if (color >= E_TEAL_COLOR && color <= E_CYAN_COLOR)
		{
			finalColour |= FOREGROUND_GREEN;
		}

		//Intensity
		if (color == E_CYAN_COLOR || color == E_LIGHT_BLUE_COLOR)
		{
			finalColour |= FOREGROUND_INTENSITY;
		}

	}
	else if (color >= E_GREEN_COLOR && color <= E_OLIVE_COLOR) // Green Base
	{
		finalColour = FOREGROUND_GREEN;

		if (color >= E_YELLOW_COLOR && color <= E_OLIVE_COLOR)
		{
			finalColour |= FOREGROUND_RED;
		}

		if (color == E_LIGHT_GREEN_COLOR || color == E_YELLOW_COLOR)
		{
			finalColour |= FOREGROUND_INTENSITY;
		}
	}
	else if (color >= E_BLACK_COLOR && color <= E_DARK_GREY_COLOR)
	{
		finalColour = 0;

		if (color == E_DARK_GREY_COLOR)
		{
			finalColour |= FOREGROUND_INTENSITY;
		}
	}
	else
	{
		finalColour = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;

		if (color == E_WHITE_COLOR)
		{
			finalColour |= FOREGROUND_INTENSITY;
		}
	}

	SetConsoleTextColor(finalColour);
}

void Color::PrintColoredText(string str)
{
	// Tokenize into Different Colour parts
	for (size_t i = 0; i < str.length(); ++i)
	{
		// If a color modifier is hit && If there is another char (the color marker) and it is a number
		if (str[i] == '%' && i + 1 < str.length())
		{
			// Find out what is the color and change into it
			COLOR_TYPE col = GetColorTypeFromSelector(str[++i]);
			
			// Invalid modifier
			if (col == E_MAX_COLORS)
			{
				// Print out normally then
				cout << str[i - 1] << str[i];
			}
			else	// Correct Modifier
			{
				SetConsoleTextColor(col);
			}
		}
		else
		{
			// Print out the text
			cout << str[i];
		}
	}

	// Reset back to defaults
	SetConsoleTextColor();
}
