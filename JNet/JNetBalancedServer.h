#pragma once

#include <vector>
#include <string>

struct _ENetHost;
struct _ENetPeer;

using std::vector;
using std::string;

namespace JNet
{
	class BalancedServer
	{
	public:
		BalancedServer();
		~BalancedServer();

	private:
		_ENetHost* m_ENetClient;
		_ENetPeer* m_ENetPeer;
		
		string m_masterServerAddress;
		unsigned short m_masterServerPort;

		string m_myName;
		string m_myAddress;
		unsigned short m_myPort;

		unsigned int playerCount;
		unsigned int playerCapacity;
		unsigned int sessionCount;

	public:
		void Initialise();
		void SetMasterServer(string address, unsigned int port);
		void SetMyConnectionInfo(string myName, string myAddress, unsigned int myPort);
		void ConnectToMasterServer();
		void Run();
	};
}

