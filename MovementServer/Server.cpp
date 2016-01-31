#include "ServerApp.h"
#include <Windows.h>

ServerApp* app;

void PacketHandlerLoop(void);
void ConsoleLoop(void);

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
}

void ConsoleLoop(void)
{
	while (true)
	{
		app->ConsoleLoop();
	}
}