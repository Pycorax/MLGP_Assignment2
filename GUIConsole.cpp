#include "GUIConsole.h"

#include "hgesprite.h"
#include "hgefont.h"

GUIConsole::GUIConsole()
	: m_maxMessages(10)
{
}


GUIConsole::~GUIConsole()
{
}

void GUIConsole::Init(int maxMessages, float consoleWidth, hgeSprite* bg, hgeFont* font)
{
	// Set the max number of messages
	m_maxMessages = maxMessages;

	// Set the console width
	m_consoleWidth = consoleWidth;

	// Clear the messages if any
	m_messageHistory.clear();

	// Set the rendering vars
	m_backgroundSprite = bg;
	m_font = font;
}

void GUIConsole::Update(bool submit, string input)
{
	m_inputBuffer = input;

	if (submit)
	{
		AddMessage(m_inputBuffer);
	}
}

void GUIConsole::Render(int posX, int posY)
{
	static const float FONT_SIZE = 0.5f;
	static const float FONT_UNIT_SIZE = 25.0f;

	// Render BG
	m_backgroundSprite->RenderStretch(posX, posY - FONT_SIZE * FONT_UNIT_SIZE * (m_maxMessages + 3 /**/),
										posX + m_consoleWidth, posY);

	// Render Message History
	m_font->SetScale(FONT_SIZE);
	int i = 4;
	for (auto msg = m_messageHistory.rbegin(); msg != m_messageHistory.rend(); ++msg)
	{
		string str = *msg;
		m_font->printf(posX, posY - FONT_UNIT_SIZE * FONT_SIZE * i, HGETEXT_LEFT, "%s", str.c_str());

		// Increment tracker
		++i;
	}

	// Render Current Input Message
	m_font->printf(posX, posY - FONT_SIZE - FONT_UNIT_SIZE, HGETEXT_LEFT, "%s", m_inputBuffer.c_str());
	m_font->SetScale(1.0f);
}

void GUIConsole::AddMessage(string str)
{
	m_messageHistory.push_back(str);
	
	// Limit History
	while (m_messageHistory.size() > m_maxMessages)
	{
		m_messageHistory.erase(m_messageHistory.begin());
	}
}

string GUIConsole::GetLastMessage(void)
{
	return m_messageHistory.back();
}
