#pragma once

// STL Includes
#include <string>
#include <queue>

// API Includes
#define NOMINMAX
#include <Windows.h>

// Other Includes
#include "ConsoleCommand.h"

// Using Directives
using std::string;
using std::queue;

class Console
{
private:
	static Console s_instance;

	// Controls if the Console is being used
	bool m_available;
	CRITICAL_SECTION m_printCSection;
	CRITICAL_SECTION m_availCSection;
	CRITICAL_SECTION m_inputCSection;

	// Controls Input Mode
	bool m_inputMode;

	// Queue of Messages to Print
	queue<string> m_printQueue;

	// Queue of Commands to Execute
	queue<ConsoleCommand> m_commandQueue;
	
public:
	static Console* Instance();
	~Console();

	ConsoleCommand Update(void);		// This will print strings that have been queued and process possible input

	void Print(string str);
	
	// Input
	void StartInput(void);
	void StopInput(void);
	bool GetInputMode(void);

	// Command Queue
	void AddCommand(string command);

private:
	Console();

	void print(string str);
	
	// Availability
	void setAvailable(bool available);
	bool getAvailable(void);
};

