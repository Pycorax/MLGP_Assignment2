#pragma once

// STL Includes
#include <string>
#include <vector>
#include <memory>

// Using Directives
using std::string;
using std::vector;
using std::auto_ptr;

// Forward Declarations
class hgeSprite;
class hgeFont;

class GUIConsole
{
public:
	static const int MAX_COMMAND_LENGTH = 50;

private:
	// For Rendering
	hgeSprite* m_backgroundSprite;
	hgeFont* m_font;
	float m_consoleWidth;

	// Max number of Messages to keep
	int m_maxMessages;

	// List of Current Messages
	vector<string> m_messageHistory;

	// Current Input
	string m_inputBuffer;

public:
	GUIConsole();
	~GUIConsole();

	void Init(int maxMessages, float consoleWidth, hgeSprite* bg, hgeFont* font);
	void Update(bool submit, string input);
	void Render(int posX, int posY);

	void AddMessage(string str);
	string GetLastMessage(void);
};

