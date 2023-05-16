#include "JNetMasterServer.h"
#include "enet/enet.h"

#include <random>

JNet::MasterServer::MasterServer()
{
	m_address = new ENetAddress();
	m_address->host = ENET_HOST_ANY;
	m_address->port = 6000;
}

JNet::MasterServer::~MasterServer()
{
}

void JNet::MasterServer::Initialize()
{
	if (enet_initialize() != 0)
		std::cout << "Enet failed to initialise." << std::endl;

	m_ENetServer = enet_host_create(m_address, 32, 2, 0, 0);

	if (m_ENetServer != nullptr)
		std::cout << "Successfully created Server" << std::endl;
}

void JNet::MasterServer::Process()
{
	// handle enet buffered packets
	ENetEvent receivedEvent;
	while (enet_host_service(m_ENetServer, &receivedEvent, 0) > 0)
	{
		switch (receivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			switch (receivedEvent.channelID)
			{
			case 0:
				InterpretUserPacket(receivedEvent);
				break;
			case 1:
				InterpretBalancedServerPacket(receivedEvent);
				break;
			default:
				break;
			}
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			break;
		}
		}
	}
}

JNet::MasterServerRedirect JNet::MasterServer::GetBalancedServer()
{
	int serverIndex = -1;

	switch (m_balanceMode)
	{
	case BalanceMode::LeastConnection:
	{
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
		m_balanceModeRRnextServer++;
		if (m_balanceModeRRnextServer >= m_balancedServers.size())
			m_balanceModeRRnextServer = 0;
		serverIndex = m_balanceModeRRnextServer;
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
	// switch on packet type
	JNetPacket* packet = (JNetPacket*)receivedEvent.packet->data;
	switch (packet->type)
	{
	case JNetPacketType::UAuth:
	{
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
			// Run Balancing function here.
			if (m_balanceMode == BalanceMode::LeastResponseTime)
			{
				std::cout << "Sending servers to Client to Ping" << std::endl;
				MakeClientPingAllServersAndConnect(receivedEvent.peer);
				break;
			}
			else
			{

				JNet::MasterServerRedirect redirectTo = GetBalancedServer();
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
	case JNetPacketType::BSRegister:
		// Add to list
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
	}
	break;
	case JNetPacketType::BSUpdate:
		// Update this BS
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
				std::cout << "Received an update event from a Balanced Server" << std::endl;
				std::cout << "\tPlayer Count: " << bsUpdate->playerCount
					<< " - Player Capacity: " << bsUpdate->playerCapacity
					<< " - Session Count: " << bsUpdate->sessionCount
					<< " - Open for new connections: " << bsUpdate->open << std::endl;
				break;
			}
		}
	}
	break;
	default:
		std::cout << "Received an unknown packet type from Balanced Server" << std::endl;
	}
}

void JNet::MasterServer::MakeClientPingAllServersAndConnect(_ENetPeer* peer)
{
	// send them the start packet and quantity of servers
	JNet::MasterServerCheckPingStart pingStart;
	pingStart.quantity = m_balancedServers.size();
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
