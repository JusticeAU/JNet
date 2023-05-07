#pragma once

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace JNet
{
	class BalancedServer
	{
	public:
		struct UserReference
		{
			string name;
			string address;
			string port;
		};
		struct GameSessionReference
		{
			string name;
			string address;
			string port;
			unsigned short players;
			unsigned short playersMax;
		};

		BalancedServer();
		~BalancedServer();

	private:
		vector<UserReference> m_Users;
		vector<GameSessionReference> m_GameSessions;

		void Update();
	};
}

