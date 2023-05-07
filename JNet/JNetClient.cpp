#include "JNetClient.h"
#include "enet/enet.h"

#include <iostream>

struct someData
{
    float seven;
    int poop;
};

JNet::Client::Client()
{
}

JNet::Client::~Client()
{
}

void JNet::Client::Initialise()
{
    if (enet_initialize() == 0)
        std::cout << "ENet Initialized!" << std::endl;
    else
        std::cout << "Failure! ENet failed to initialize" << std::endl;
}

void JNet::Client::SetMasterServer(string address, unsigned int port)
{
	m_masterServer.address = address;
	m_masterServer.port = port;
}

void JNet::Client::Connect()
{
	ConnectToMasterServer();
}

void JNet::Client::ConnectToMasterServer()
{
	m_ENetClient = enet_host_create(nullptr, 1, 2, 0, 0);
	
    ENetAddress address;
    enet_address_set_host(&address, m_masterServer.address.c_str());
    address.port = m_masterServer.port;

    ENetPeer* peer;
	peer = enet_host_connect(m_ENetClient, &address, 2, 0);

    while (true) // client loop
    {
        ENetEvent receivedEvent; // the variable to place the info in.
        ENetPeer* serverPeer = nullptr;

        // While more information awats us...
        while (enet_host_service(m_ENetClient, &receivedEvent, 100) > 0)
        {
            // Process
            std::cout << "client received a packet!" << std::endl;
            switch (receivedEvent.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "We have confirmed connection with the server." << std::endl;
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "We have disconnected from the server." << std::endl;
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::cout << "We received a user-defined packet!" << std::endl;
                break;
            }
            default:
                std::cout << "some other data received" << std::endl;
                break;
            }
        }
    }
}
