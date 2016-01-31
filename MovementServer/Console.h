#pragma once

// STL Includes
#include <string>
#include <queue>

// API Includes
#include <Windows.h>

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

	// Queue of Messages to Print
	queue<string> m_printQueue;
	
public:
	static Console* Instance();
	~Console();

	void Update(void);		// This will print strings that have been queued

	void Print(string str);

private:
	Console();

	void print(string str);
	
	// Availability
	void setAvailable(bool available);
	bool getAvailable(void);

};

