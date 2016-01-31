#ifndef COLOR_SYSTEM_H
#define COLOR_SYSTEM_H

// API Includes
#include <Windows.h>

// STL Includes
#include <string>

// Using Directives
using std::string;

namespace Color
{
	enum COLOR_TYPE
	{
		//Red Base
		E_RED_COLOR,
		E_PURPLE_COLOR,
		//Need intensity
		E_BRIGHT_RED_COLOR,
		E_MAGENTA_COLOR,

		//Blue Base
		E_BLUE_COLOR,
		E_TEAL_COLOR,
		//Need Intensity
		E_CYAN_COLOR,
		E_LIGHT_BLUE_COLOR,

		//Green Base
		E_GREEN_COLOR,
		//Need Intensity
		E_LIGHT_GREEN_COLOR,
		E_YELLOW_COLOR,
		E_OLIVE_COLOR,

		//All Base
		E_GREY_COLOR,
		E_WHITE_COLOR,

		//0 Base
		E_BLACK_COLOR,
		E_DARK_GREY_COLOR,

		E_MAX_COLORS
	};

	const char COLOR_MARKER = '%';
	const char COLOR_SELECTOR[E_MAX_COLORS] =
	{
		//Red Base
		NULL,
		'1',
		//Need Intensity
		'2',
		'3',
		//Blue Base
		'4',
		'5',
		//Need Intensity
		'6',
		'7',
		//Green Base
		'8',
		//Need Intensity
		'9',
		'-',
		'=',
		//All Base
		'0',
		NULL,
		//0 Base
		NULL,
		NULL
	};

	COLOR_TYPE GetColorTypeFromSelector(char ch);
	void SetConsoleTextColor(WORD attrib);
	void SetConsoleTextColor(COLOR_TYPE color = E_GREY_COLOR);
	void PrintColoredText(string str);

}

#endif