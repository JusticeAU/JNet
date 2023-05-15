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
    if (enet_initialize() == 0)
        std::cout << "ENet Initialized!" << std::endl;
    else
        std::cout << "Failure! ENet failed to initialize" << std::endl;
}

void JNet::GameSession::Process()
{
	// Balanced Server Updates
	ENetEvent BSreceivedEvent;
	while (enet_host_service(m_ENetBalancedServerClient, &BSreceivedEvent, 100) > 0)
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
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			break;
		}
		}
	}

	// Game Session Host Updates
	ENetEvent GSreceivedEvent;
	while (enet_host_service(m_ENetGameSessionClient, &GSreceivedEvent, 100) > 0)
	{
		switch (GSreceivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			std::cout << "A client connected" << std::endl;
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
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
