#include "Console.h"

// STL Includes
#include <iostream>
#include <conio.h>
#include <limits>

// Other Includes
#include "ColorSystem.h"

// Using Directives
using std::cout;
using std::cin;
using std::endl;

Console Console::s_instance;

Console::Console()
	: m_available(true)
	, m_inputMode(false)
{
	InitializeCriticalSection(&m_printCSection);
	InitializeCriticalSection(&m_availCSection);
	InitializeCriticalSection(&m_inputCSection);
}

void Console::print(string str)
{
	setAvailable(false);
	EnterCriticalSection(&m_printCSection);

	// Highlight \ colours
	if (str.length() > 1 && str[0] == '\\')
	{
		Color::SetConsoleTextColor(Color::E_BRIGHT_RED_COLOR);
		cout << str.substr(1, str.npos);
		Color::SetConsoleTextColor();
	}
	else
	{
		cout << str;
	}
	
	LeaveCriticalSection(&m_printCSection);
	setAvailable(true);
}

void Console::setAvailable(bool available)
{
	EnterCriticalSection(&m_availCSection);
	m_available = available;
	LeaveCriticalSection(&m_availCSection);
}

bool Console::getAvailable(void)
{
	bool avail = false;
	EnterCriticalSection(&m_availCSection);
	avail = m_available;
	LeaveCriticalSection(&m_availCSection);
	return avail;
}

Console * Console::Instance()
{
	return &s_instance;
}

Console::~Console()
{
	DeleteCriticalSection(&m_inputCSection);
	DeleteCriticalSection(&m_printCSection);
	DeleteCriticalSection(&m_availCSection);
}

ConsoleCommand Console::Update(void)
{
	// Input
	if (GetInputMode())
	{
		// Process the command
		string inputstring;
		getline(std::cin, inputstring);

		// Block input
		StopInput();

		return ConsoleCommand::GetCommandFromString(inputstring);
	}
	else if (m_printQueue.size() > 0 && getAvailable())		// If we are able to get a slice of time to print something
	{
		// Print
		print(m_printQueue.front());
		// Remove it from queue
		m_printQueue.pop();
	}

	// Process commands in the queue, sent in externally
	if (m_commandQueue.size() > 0)
	{
		ConsoleCommand cmd = m_commandQueue.back();
		m_commandQueue.pop();
		return cmd;
	}

	return ConsoleCommand();		// Return an empty one
}

void Console::Print(string str)
{
	if (getAvailable() && !GetInputMode())
	{
		print(str);
	}
	else
	{
		// Print later
		m_printQueue.push(str);
	}
}

void Console::StartInput(void)
{
	EnterCriticalSection(&m_printCSection);
	m_inputMode = true;
	LeaveCriticalSection(&m_printCSection);
}

void Console::StopInput(void)
{
	EnterCriticalSection(&m_printCSection);
	m_inputMode = false;
	LeaveCriticalSection(&m_printCSection);
}

bool Console::GetInputMode(void)
{
	bool inputMode = false;
	EnterCriticalSection(&m_inputCSection);
	inputMode = m_inputMode;
	LeaveCriticalSection(&m_inputCSection);
	return inputMode;
}

void Console::AddCommand(string command)
{
	ConsoleCommand cmd = ConsoleCommand::GetCommandFromString(command);

	if (cmd.command != ConsoleCommand::C_TOTAL)
	{
		m_commandQueue.push(cmd);
	}
}
