#pragma once

// STL Includes
#include <string>
#include <vector>
#include <ctime>

// API Includes
#include "BitStream.h"

// Other Includes
#include "Ball.h"

// Using Directives
using std::string;
using std::vector;

class Room
{
public:
	enum TEAM_TYPE
	{
		TEAM_BLUE,
		TEAM_RED,
		TOTAL_TEAMS
	};

	static const unsigned MAX_ROOM_NAME_LENGTH = 30;

private:
	// Room Details
	string name;
	int id;
	vector<int> connectedIDs;				// Stores the IDs of the clients that are connected to this room

	// Game Play 
	vector<int> teamList[TOTAL_TEAMS];		// Stores the IDs of teams
	int scores[TOTAL_TEAMS];				// Stores the score
	Ball* ball;					// Ball pointer

public:
	Room(Ball* ballPtr = new Ball, string _name = "", int _id = 0) : id(_id)
	{
		name = trim(_name, MAX_ROOM_NAME_LENGTH);

		// Initialize the scores
		for (auto& score : scores)
		{
			score = 0;
		}

		ball = ballPtr;

		// Initialize the ball
		ResetBall();
	}

	void Update(double dt)
	{
		ball->Update(dt);
	}

	void Exit(void)
	{
		delete ball;
		ball = nullptr;
	}

	void AddUser(int userID, TEAM_TYPE team = TOTAL_TEAMS)
	{
		if (!isConnected(userID))
		{
			//  Add to connectedIDs
			connectedIDs.push_back(userID);

			// Is a team provided for us?
			if (team == TOTAL_TEAMS)	// No?
			{
				// Determine the team to throw the player ourselves
				if (teamList[TEAM_BLUE].size() > teamList[TEAM_RED].size())
				{
					teamList[TEAM_RED].push_back(userID);
				}
				else if (teamList[TEAM_BLUE].size() < teamList[TEAM_RED].size())
				{
					teamList[TEAM_BLUE].push_back(userID);
				}
				else
				{
					// If they have an equal number, randomly pick one
					srand(time_t(NULL));
					if (rand() % 2)
					{
						teamList[TEAM_RED].push_back(userID);
					}
					else
					{
						teamList[TEAM_BLUE].push_back(userID);
					}
				}
			}
			else	// Yes? We use what we're given
			{
				teamList[team].push_back(userID);
			}
		}
	}

	void RemoveUser(int userID)
	{
		if (isConnected(userID))
		{
			// Find user and Remove from connectedIDs
			for (auto u = connectedIDs.begin(); u != connectedIDs.end(); ++u)
			{
				if (*u == userID)
				{
					connectedIDs.erase(u);
					break;
				}
			}

			// Find user and Remove from teamList
			// -- Get the user's team
			TEAM_TYPE team = GetUserTeam(userID);
			// -- Find and remove from teamList
			for (auto user = teamList[team].begin(); user != teamList[team].end(); ++user)
			{
				if (*user == userID)
				{
					teamList[team].erase(user);
					break;
				}
			}
		}
	}

	void ClearRoom(void)
	{
		connectedIDs.clear();
	}

	string GetName(void) const
	{
		return name;
	}

	int GetID(void) const
	{
		return id;
	}

	vector<int> GetConnectedIDs(void) const
	{
		return connectedIDs;
	}

	vector<int> GetTeamList(TEAM_TYPE team) const
	{
		return teamList[team];
	}

	int GetScore(TEAM_TYPE team) const
	{
		return scores[team];
	}

	TEAM_TYPE GetUserTeam(int userID) const
	{
		// Check for each team
		for (int i = 0; i < TOTAL_TEAMS; ++i)
		{
			// Get the team
			auto team = teamList[i];

			// Check for each user
			for (auto user : team)
			{
				if (user == userID)
				{
					return static_cast<TEAM_TYPE>(i);
				}
			}
		}

		return TOTAL_TEAMS;
	}

	void ResetBall()
	{
		static const int WINDOW_WIDTH = 800;
		static const int WINDOW_HEIGHT = 600;
		ball->setLocation(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f, 0.0f);
	}

	Ball* GetBall()
	{
		return ball;
	}

	void Send(RakNet::BitStream* bs, MyMsgIDs type)
	{
		ball->SendObject(bs, type);
	}
	void Receive(RakNet::BitStream* bs, MyMsgIDs type)
	{
		ball->RecvObject(bs, type);
	}

private:
	bool isConnected(int userID)		// Checks if a user is connected
	{
		for (auto u : connectedIDs)
		{
			if (userID == u)
			{
				return true;
			}
		}

		return false;
	}

	string trim(string str, int maxSize)
	{
		if (str.length() > maxSize)
		{
			return str.substr(0, maxSize);
		}
		else
		{
			return str;
		}
	}
};

