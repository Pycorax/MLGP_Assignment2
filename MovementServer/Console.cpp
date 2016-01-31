#include "Console.h"

// STL Includes
#include <iostream>

// Using Directives
using std::cout;
using std::cin;
using std::endl;

Console Console::s_instance;

Console::Console()
	: m_available(true)
{
	InitializeCriticalSection(&m_printCSection);
	InitializeCriticalSection(&m_availCSection);
}

void Console::print(string str)
{
	setAvailable(false);
	EnterCriticalSection(&m_printCSection);
	cout << str;
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
	DeleteCriticalSection(&m_printCSection);
	DeleteCriticalSection(&m_availCSection);
}

void Console::Update(void)
{
	// If we are able to get a slice of time to print something
	if (m_printQueue.size() > 0 && getAvailable())
	{
		// Print
		print(m_printQueue.front());
		// Remove it from queue
		m_printQueue.pop();
	}
}

void Console::Print(string str)
{
	if (m_available)
	{
		print(str);
	}
	else
	{
		// Print later
		m_printQueue.push(str);
	}
}
