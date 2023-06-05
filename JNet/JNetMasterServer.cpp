#include "JNetMasterServer.h"
#include "enet/enet.h"

#include <random>

#include <fstream>
#include <string>
#include "string_split.h"

using namespace std::chrono;

JNet::MasterServer::MasterServer()
{

}

JNet::MasterServer::~MasterServer()
{
}

void JNet::MasterServer::Start()
{
	m_ENetClientAddress = new ENetAddress();
	m_ENetClientAddress->host = ENET_HOST_ANY;
	m_ENetClientAddress->port = m_ENetClientPort;

	m_ENetBSAddress = new ENetAddress();
	m_ENetBSAddress->host = ENET_HOST_ANY;
	m_ENetBSAddress->port = m_ENetBSPort;

	if (enet_initialize() != 0)
		std::cout << "Enet failed to initialise." << std::endl;

	m_ENetClientHost = enet_host_create(m_ENetClientAddress, 32, 2, 0, 0);
	m_ENetBSHost = enet_host_create(m_ENetBSAddress, 32, 2, 0, 0);

	if (m_ENetClientHost != nullptr)
		std::cout << "Successfully created Server" << std::endl;
}

void JNet::MasterServer::LoadGeoIPDatabase(string filename)
{
	milliseconds msStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	std::cout << "Loading Geo IP Database" << std::endl;
	std::ifstream file(filename.c_str()); 
	if (!file.is_open())
	{
		std::cout << "Failed to load database from: " << filename << std::endl;
		return;
	}

	int entries = 0;
	while (!file.eof())
	{
		entries++;
		std::string line;
		std::getline(file, line);
		if (line.find(',') == string::npos) // invalid data, bail out
			break;
		// get all thee data we'll need.
		std::string* lineSplit = string_split(line, ',');
		std::string* firstSplit = string_split(lineSplit[0], '.');
		std::string* secondSplit = string_split(lineSplit[1], '.');
		std::string country = lineSplit[2];
		octet IPRange[4];
		for (int i = 0; i < 4; i++)
		{
			IPRange[i].min = atoi(firstSplit[i].c_str());
			IPRange[i].max = atoi(secondSplit[i].c_str());
		}
		IPRange[3].country = country; // load country code in to last octet range

		// Track country codes
		bool countryExists = false;
		for (int i = 0; i < m_geoRoutingCountryCodes.size(); i++)
		{
			if (m_geoRoutingCountryCodes[i] == country)
			{
				countryExists = true;
				break;
			}
		}
		if (!countryExists)
			m_geoRoutingCountryCodes.push_back(country);

		delete[] lineSplit;
		delete[] firstSplit;
		delete[] secondSplit;

		bool placed = false;
		for (int i = 0; i < m_geoRoutingIPDatabase.size(); i++)			// FIRST
		{
			if (m_geoRoutingIPDatabase[i].min == IPRange[0].min)
			{
				octet* first = &m_geoRoutingIPDatabase[i];
				for (int j = 0; j < first->next.size(); j++)			// SECOND
				{
					if (first->next[j].min == IPRange[1].min)
					{
						octet* second = &first->next[j];
						for (int k = 0; k < second->next.size(); k++)	// THIRD
						{
							if (second->next[k].min == IPRange[2].min)
							{
								octet* third = &second->next[k];
								// Just add it							// FORTH
								third->next.push_back(IPRange[3]);
								placed = true;
								break;
							}
						}
						if (!placed)
						{
							second->next.push_back(IPRange[2]);
							second->next.back().next.push_back(IPRange[3]);
							placed = true;
						}

					}
					if (placed)
						break;
				}
				if (!placed)
				{
					first->next.push_back(IPRange[1]);
					first->next.back().next.push_back(IPRange[2]);
					first->next.back().next.back().next.push_back(IPRange[3]);
					placed = true;
				}
			}
			if (placed)
				break;
		}
		if (!placed)
		{
			m_geoRoutingIPDatabase.push_back(IPRange[0]);
			m_geoRoutingIPDatabase.back().next.push_back(IPRange[1]);
			m_geoRoutingIPDatabase.back().next.back().next.push_back(IPRange[2]);
			m_geoRoutingIPDatabase.back().next.back().next.back().next.push_back(IPRange[3]);
			placed = true;
		}
	}
	file.close();
	milliseconds msEnd = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	milliseconds duration = msEnd - msStart;
	std::cout << "Loaded " << std::to_string(entries) << " IP ranges across " << std::to_string(m_geoRoutingCountryCodes.size()) << " country codes in " << duration.count() << " milliseconds." << std::endl;
}

void JNet::MasterServer::Process()
{
	// handle Client Connections
	ENetEvent receivedEvent;
	while (enet_host_service(m_ENetClientHost, &receivedEvent, 0) > 0)
	{
		switch (receivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			if (m_ClientConnectCallBack) m_ClientConnectCallBack(&receivedEvent);
	
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			InterpretUserPacket(receivedEvent);

			if (m_ClientPacketCallBack) m_ClientPacketCallBack(&receivedEvent);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			if (m_ClientDisconnectCallBack) m_ClientDisconnectCallBack(&receivedEvent);
			break;
		}
		default:
			break;
		}
	}

	// Handle Balanced Server Connections
	while (enet_host_service(m_ENetBSHost, &receivedEvent, 0) > 0)
	{
		switch (receivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			if (m_BalancedServerConnectCallBack) m_BalancedServerConnectCallBack(&receivedEvent);
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			InterpretBalancedServerPacket(receivedEvent);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{

			for (int i = 0; i < m_balancedServers.size(); i++)
			{
				if (m_balancedServers[i].peer == receivedEvent.peer)
				{
					m_balancedServers.erase(m_balancedServers.begin() + i);
					break;
				}
			}

			if (m_BalancedServerDisconnectCallBack) m_BalancedServerDisconnectCallBack(&receivedEvent);

			break;
		}
		default:
			break;
		}
	}
}

JNet::MasterServerRedirect JNet::MasterServer::GetBalancedServer(_ENetPeer* peer)
{
	int serverIndex = -1;

	switch (m_balanceMode)
	{
	case BalanceMode::LeastConnection:
	{
		std::cout << "Sending user to server with least amount of users" << std::endl;
		int leastConnections = INT_MAX;
		for (int i = 0; i < m_balancedServers.size(); i++)
		{
			if (m_balancedServers[i].playerCount < leastConnections)
			{
				leastConnections = m_balancedServers[i].playerCount;
				serverIndex = i;
			}
		}
		break;
	}
	case BalanceMode::RoundRobin:
	{
		std::cout << "Sending user to next server in round robin rotation" << std::endl;
		m_balanceModeRRnextServer++;
		if (m_balanceModeRRnextServer >= m_balancedServers.size())
			m_balanceModeRRnextServer = 0;
		serverIndex = m_balanceModeRRnextServer;
		break;
	}
	case BalanceMode::GeoLocation:
	{
		std::cout << "Sending user to server based on their IP GEO location" << std::endl;
		unsigned int ip = peer->address.host;
		unsigned char one, two, three, four;
		one = ip;
		two = ip >> 8;
		three = ip >> 16;
		four = ip >> 24;
		string country = GetCountryFromIP(one, two, three, four);
		std::cout << "Detected user country code: " << country << std::endl;
		serverIndex = GetIndexOfBalancedServerForCountry(country);
		break;
	}
	}

	// Build our packet and return it.
	JNet::MasterServerRedirect redirectTo;
	strcpy_s(redirectTo.name, m_balancedServers[serverIndex].name.c_str());
	strcpy_s(redirectTo.hostname, m_balancedServers[serverIndex].address.c_str());
	redirectTo.port = m_balancedServers[serverIndex].port;
	return redirectTo;
}

void JNet::MasterServer::InterpretUserPacket(_ENetEvent& receivedEvent)
{
	JNetPacket* packet = (JNetPacket*)receivedEvent.packet->data;
	switch (packet->type)
	{
	case JNetPacketType::ClientAuth:
	{
		std::cout << "A User has connected and authenticated" << std::endl;
		ENetPacket* infoPacket;
		ENetPeer* serverPeer = receivedEvent.peer;

		if (m_balancedServers.size() == 0)
		{
			JNet::ErrorMessage errorMsg;
			strcpy_s(errorMsg.message, "No Servers available. Please try again later");
			infoPacket = enet_packet_create(&errorMsg, sizeof(JNet::ErrorMessage), ENET_PACKET_FLAG_RELIABLE);
		}
		else
		{
			// LeastResponseTime is more complex that simply returning a server, so handled via exception here.
			if (m_balanceMode == BalanceMode::LeastResponseTime)
			{
				std::cout << "Sending servers to Client to Ping" << std::endl;
				MakeClientPingAllServersAndConnect(receivedEvent.peer);
				break;
			}
			else
			{

				JNet::MasterServerRedirect redirectTo = GetBalancedServer(receivedEvent.peer);
				infoPacket = enet_packet_create(&redirectTo, sizeof(JNet::MasterServerRedirect), ENET_PACKET_FLAG_RELIABLE);
				std::cout << "Redirect user to " << redirectTo.name << std::endl; // should show username here for verbosity.
			}
		}

		enet_peer_send(serverPeer, 0, infoPacket);
		enet_peer_disconnect_later(serverPeer, 0);
		break;
	}
	default:
	{
		std::cout << "We received some other data!" << std::endl;
		break;
	}
	}
}

void JNet::MasterServer::InterpretBalancedServerPacket(_ENetEvent& receivedEvent)
{
	JNetPacket* packet = (JNetPacket*)receivedEvent.packet->data;
	switch (packet->type)
	{
	case JNetPacketType::BSRegister: // Add to list
	{
		BalancedServerRegister* bsRegister = (BalancedServerRegister*)receivedEvent.packet->data;
		std::cout << "Received a register event from a Balanced Server" << std::endl;
		std::cout << "\tName: " << bsRegister->name << " - Hostname: " << bsRegister->hostname << " - Port: " << bsRegister->port << std::endl;
		BalancedServerReference server;
		server.peer = receivedEvent.peer;
		server.name = bsRegister->name;
		server.address = bsRegister->hostname;
		server.port = bsRegister->port;
		server.playerCount = bsRegister->playerCount;
		server.playerCapacity = bsRegister->playerCapacity;
		server.sessionCount = bsRegister->sessionCount;
		server.open = bsRegister->open;
		m_balancedServers.push_back(server);
		break;
	}
	case JNetPacketType::BSUpdate:		// Update this BS
	{
		BalancedServerUpdate* bsUpdate = (BalancedServerUpdate*)receivedEvent.packet->data;

		for (BalancedServerReference& bs : m_balancedServers)
		{
			if (bs.peer == receivedEvent.peer)
			{
				bs.playerCount = bsUpdate->playerCount;
				bs.playerCapacity = bsUpdate->playerCapacity;
				bs.sessionCount = bsUpdate->sessionCount;
				bs.open = bsUpdate->open;
				/*std::cout << "Received an update event from a Balanced Server" << std::endl;
				std::cout << "\tPlayer Count: " << bsUpdate->playerCount
					<< " - Player Capacity: " << bsUpdate->playerCapacity
					<< " - Session Count: " << bsUpdate->sessionCount
					<< " - Open for new connections: " << bsUpdate->open << std::endl;*/
				break;
			}
		}
		break;
	}
	case JNetPacketType::BSAddCountryCode:
	{
		std::cout << "Received country code from balanced server" << std::endl;
		BalancedServerAddCountryCode* bsCountryCode = (BalancedServerAddCountryCode*)receivedEvent.packet->data;
		for (int i = 0; i < m_balancedServers.size(); i++)
		{
			if (m_balancedServers[i].peer == receivedEvent.peer)
			{
				string code = bsCountryCode->name;
				m_balancedServers[i].geoRoutingCountryCodes.push_back(code);
				break;
			}
		}
		break;
	}
	default:
		std::cout << "Received an unknown packet type from Balanced Server" << std::endl;
	}
}

void JNet::MasterServer::MakeClientPingAllServersAndConnect(_ENetPeer* peer)
{
	// send them the start packet and quantity of servers
	JNet::MasterServerCheckPingStart pingStart;
	pingStart.quantity = (int)m_balancedServers.size();
	ENetPacket* startPacket = enet_packet_create(&pingStart, sizeof(MasterServerCheckPingStart), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, startPacket);
	
	// loop through and send each server as a individual packet.
	for (BalancedServerReference server : m_balancedServers)
	{
		JNet::MasterServerCheckPingServer pingServer;
		strcpy_s(pingServer.name, server.name.c_str());
		strcpy_s(pingServer.hostname, server.address.c_str());
		pingServer.port = server.port;

		ENetPacket* serverPacket = enet_packet_create(&pingServer, sizeof(MasterServerCheckPingServer), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, serverPacket);
	}
	
}

std::string JNet::MasterServer::GetCountryFromIP(int first, int second, int third, int forth)
{
	nanoseconds msStart = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

	for (int i = 0; i < m_geoRoutingIPDatabase.size(); i++)
	{
		int min = m_geoRoutingIPDatabase[i].min;
		int max = m_geoRoutingIPDatabase[i].max;
		if (first >= min && first <= max)
		{
			octet& firstOctect = m_geoRoutingIPDatabase[i];
			std::vector<octet>& secondRange = m_geoRoutingIPDatabase[i].next;
			for (int j = 0; j < secondRange.size(); j++)
			{
				int min = secondRange[j].min;
				int max = secondRange[j].max;
				if (second >= min && second <= max)
				{
					octet& secondOctect = secondRange[j];
					std::vector<octet>& thirdRange = secondRange[j].next;
					for (int k = 0; k < thirdRange.size(); k++)
					{
						int min = thirdRange[k].min;
						int max = thirdRange[k].max;
						if (third >= min && third <= max)
						{
							octet& thirdOctect = thirdRange[k];
							std::vector<octet>& forthRange = thirdRange[k].next;
							for (int l = 0; l < forthRange.size(); l++)
							{
								int min = forthRange[l].min;
								int max = forthRange[l].max;
								if (forth >= min && forth <= max)
								{
									nanoseconds msEnd = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
									nanoseconds duration = msEnd - msStart;
									std::cout << "Searched IP Address in " << duration.count() << " nanoseconds." << std::endl;
									return forthRange[l].country;
								}
							}
						}
					}
				}
			}
		}
	}

	nanoseconds msEnd = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
	nanoseconds duration = msEnd - msStart;
	std::cout << "Searched IP Address in " << duration.count() << " nanoseconds." << std::endl;
	return "DEFAULT";
}

int JNet::MasterServer::GetIndexOfBalancedServerForCountry(string code)
{
	for (int i = 0; i < m_balancedServers.size(); i++)
	{
		for (string countryCode : m_balancedServers[i].geoRoutingCountryCodes)
		{
			if (code == countryCode)
				return i;
		}
	}

	return 0;
}
