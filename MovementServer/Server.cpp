#include "ServerApp.h"
#include <Windows.h>
#include <iostream>

using std::cout;
using std::endl;

ServerApp* app;

void PacketHandlerLoop(void);
void ConsoleLoop(void);
void GameLoop(void);

int main()
{
	// Create ServerApp
	app = new ServerApp();

	// Thread Handle
	HANDLE threadHandle[ServerApp::THREAD_TOTAL];
	// Thread IDs
	DWORD ThreadID[ServerApp::THREAD_TOTAL];

	// Starting of Threads
	threadHandle[ServerApp::THREAD_PACKET_HANDLER] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PacketHandlerLoop, (LPVOID)NULL, 0, &ThreadID[ServerApp::THREAD_PACKET_HANDLER]);
	threadHandle[ServerApp::THREAD_CONSOLE] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConsoleLoop, (LPVOID)NULL, 0, &ThreadID[ServerApp::THREAD_CONSOLE]);
	threadHandle[ServerApp::THREAD_GAME] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GameLoop, (LPVOID)NULL, 0, &ThreadID[ServerApp::THREAD_GAME]);

	// Wait for everything to end first
	WaitForMultipleObjects(ServerApp::THREAD_TOTAL, threadHandle, TRUE, INFINITE);

	// Delete the ServerApp
	delete app;

	return 0;
}

void PacketHandlerLoop(void)
{
	while (true)
	{
		app->PacketHandlerLoop();
	}

	cout << "PacketHandler Thread Ended!" << endl;
}

void ConsoleLoop(void)
{
	while (true)
	{
		app->ConsoleLoop();
	}

	cout << "Console Thread Ended!" << endl;
}

void GameLoop(void)
{
	while (true)
	{
		app->GameLoop();
	}

	cout << "Game Thread Ended!" << endl;
}
