#pragma once

// STL Includes
#include <vector>
#include <string>

// Using Directives
using std::vector;
using std::string;

class ConsoleCommand
{
public: // Types and Variables
	enum COMMAND_TYPE
	{
		C_DEBUG_PRINT_ROOMS,
		C_TOTAL
	};

	// List of Commands in string form
	static const string COMMAND_LIST[C_TOTAL];

	// Command properties
	COMMAND_TYPE command;
	std::vector<string> params;

public:	// Functions
	ConsoleCommand();
	~ConsoleCommand();

	// Function to obtain a parsed command
	static ConsoleCommand GetCommandFromString(string message);
};