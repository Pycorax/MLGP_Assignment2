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
		C_DEBUG_PRINT_GOALS,
		C_CREATE_ROOM,
		C_SET_CONNECTIONS,
		C_TOTAL
	};

	// List of Commands in string form
	static const string COMMAND_LIST[C_TOTAL];

	// Command properties
	COMMAND_TYPE command;
	vector<string> params;

public:	// Functions
	ConsoleCommand();
	~ConsoleCommand();

	// Function to obtain a parsed command
	static ConsoleCommand GetCommandFromString(string message);
	static string ConcatParamList(vector<string> params);			// Function for concatenating a list of params into a single string
};