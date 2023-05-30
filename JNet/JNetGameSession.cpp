#include "JNetGameSession.h"
#include "JNetPackets.h"

#include <iostream>
#include "enet/enet.h"

JNet::GameSession::GameSession()
{
}

JNet::GameSession::~GameSession()
{
}

void JNet::GameSession::Initialise()
{
    if (enet_initialize() != 0)
		std::cout << "Enet failed to initialise." << std::endl;
}

void JNet::GameSession::Process()
{
	// Balanced Server Updates
	ENetEvent BSreceivedEvent;
	while (enet_host_service(m_ENetBalancedServerClient, &BSreceivedEvent, 0) > 0)
	{
		switch (BSreceivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			GameSessionRegister GSRegister;
			strcpy_s(GSRegister.name, m_myName.c_str());
			strcpy_s(GSRegister.hostname, m_myAddress.c_str());
			GSRegister.port = m_myPort;
			ENetPacket* packet = enet_packet_create(&GSRegister, sizeof(GameSessionRegister), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(m_ENetBalancedServerPeer, 0, packet);

			if (m_BalancedServerConnectCallBack)
				m_BalancedServerConnectCallBack(&BSreceivedEvent);

			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			if (m_BalancedServerPacketCallBack)
				m_BalancedServerPacketCallBack(&BSreceivedEvent);

			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{

			if (m_BalancedServerDisconnectCallBack)
				m_BalancedServerDisconnectCallBack(&BSreceivedEvent);

			break;
		}
		}
	}

	// Game Session Host Updates
	ENetEvent GSreceivedEvent;
	while (enet_host_service(m_ENetGameSessionClient, &GSreceivedEvent, 0) > 0)
	{
		switch (GSreceivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			std::cout << "A client connected" << std::endl;

			if (m_ClientConnectCallBack)
				m_ClientConnectCallBack(&GSreceivedEvent);

			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			if (m_ClientPacketCallBack)
				m_ClientPacketCallBack(&GSreceivedEvent);

			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			if (m_ClientDisconnectCallBack)
				m_ClientDisconnectCallBack(&GSreceivedEvent);

			break;
		}
		}

		
	}
}

void JNet::GameSession::SetBalancedServer(string address, unsigned int port)
{
    m_balancedServerAddress = address;
    m_balancedServerPort = port;
}

void JNet::GameSession::SetMyConnectionInfo(string name, string address, unsigned int port)
{
    m_myName = name;
    m_myAddress = address;
    m_myPort = port;
}

void JNet::GameSession::ConnectToBalancedServer()
{
    m_ENetBalancedServerClient = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, m_balancedServerAddress.c_str());
    address.port = m_balancedServerPort;

    m_ENetBalancedServerPeer = enet_host_connect(m_ENetBalancedServerClient, &address, 2, 0);
}

void JNet::GameSession::CheckInWithBalancedServer()
{
}

void JNet::GameSession::UpdateBalancedServer()
{
}

void JNet::GameSession::StartMyHost()
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = m_myPort;

	m_ENetGameSessionClient = enet_host_create(&address, 32, 2, 0, 0);
}
