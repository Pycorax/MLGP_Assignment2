#include "ConsoleCommand.h"

const string ConsoleCommand::COMMAND_LIST[C_TOTAL] =
{
	"printrooms"
};

ConsoleCommand::ConsoleCommand()
	: command(C_TOTAL)
{
}

ConsoleCommand::~ConsoleCommand()
{
}

ConsoleCommand ConsoleCommand::GetCommandFromString(string message)
{
	ConsoleCommand result;

	// Check if it is a valid command string
	if (message.size() < 0)
	{
		return result;
	}

	/*
	* Obtaining the Command
	*/
	// Pluck out the command string
	string commandMsg;
	for (size_t i = 0; i < message.size(); ++i)
	{
		if (message[i] == ' ')
		{
			break;
		}
		else
		{
			commandMsg += message[i];
		}
	}
	// Check which command it is
	for (size_t i = 0; i < C_TOTAL; i++)
	{
		if (commandMsg == COMMAND_LIST[i])
		{
			result.command = static_cast<COMMAND_TYPE>(i);
		}
	}

	/*
	* Obtaining the Command Params
	*/
	// Try to Extract the param portion out
	string paramString;
	try
	{
		paramString = message.substr(commandMsg.size() + 2 /*For the space between command and the params and the '/' at the start*/);
	}
	catch (std::out_of_range e)
	{
		// If there is no params, don't tokenize
		return result;
	}

	// Tokenize the params
	int prevTokenIndex = 0;
	for (size_t i = 0; i < paramString.length(); ++i)
	{
		// If a white space is hit
		if (isspace(paramString[i]))
		{
			// This must mean the previous block is a token
			result.params.push_back(paramString.substr(prevTokenIndex, i - prevTokenIndex));
			prevTokenIndex = i + 1 /*+1 to skip the whitespace*/;
		}
	}

	// Add the final token
	result.params.push_back(paramString.substr(prevTokenIndex));

	return result;
}
