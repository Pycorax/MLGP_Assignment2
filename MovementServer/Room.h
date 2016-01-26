#pragma once

// STL Includes
#include <string>
#include <vector>

// Using Directives
using std::string;
using std::vector;

class Room
{
public:
	static const unsigned MAX_ROOM_NAME_LENGTH = 30;

private:
	string name;
	int id;
	vector<int> connectedIDs;		// Stores the IDs of the clients that are connected to this roomRoom(string _name = "") : name(_name) {}

public:
	Room(string _name = "", int _id = 0) : name(_name), id(_id)
	{
	}

	void AddUser(int userID)
	{
		if (!isConnected(userID))
		{
			connectedIDs.push_back(userID);
		}
	}
	void RemoveUser(int userID)
	{
		if (isConnected(userID))
		{
			for (auto u = connectedIDs.begin(); u != connectedIDs.end(); ++u)
			{
				if (*u == userID)
				{
					connectedIDs.erase(u);
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
};

