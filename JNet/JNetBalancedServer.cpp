#include "JNetBalancedServer.h"
#include "enet/enet.h"
#include "JNetPackets.h"

#include <iostream>

JNet::BalancedServer::BalancedServer()
{
}

JNet::BalancedServer::~BalancedServer()
{
}

void JNet::BalancedServer::Initialise()
{
    if (enet_initialize() == 0)
        std::cout << "ENet Initialized!" << std::endl;
    else
        std::cout << "Failure! ENet failed to initialize" << std::endl;
}

void JNet::BalancedServer::SetMasterServer(string address, unsigned int port)
{
    m_masterServerAddress = address;
    m_masterServerPort = port;
}

void JNet::BalancedServer::SetMyConnectionInfo(string myName, string myAddress, unsigned int myPort)
{
    m_myName = myName;
    m_myAddress = myAddress;
    m_myPort = myPort;
}

void JNet::BalancedServer::ConnectToMasterServer()
{
    m_ENetClient = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, m_masterServerAddress.c_str());
    address.port = m_masterServerPort;

    
    m_ENetPeer = enet_host_connect(m_ENetClient, &address, 2, 0);
}

void JNet::BalancedServer::Run()
{
    while (true) // client loop
    {
        ENetEvent receivedEvent; // the variable to place the info in.

        // While more information awats us...
        while (enet_host_service(m_ENetClient, &receivedEvent, 100) > 0)
        {
            // Process
            std::cout << "client received a packet!" << std::endl;
            switch (receivedEvent.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "We have confirmed connection with the server. Registering self." << std::endl;
                {
                    JNet::BalancedServerRegister registerPacket;
                    strcpy_s(registerPacket.name, m_myName.c_str());
                    strcpy_s(registerPacket.hostname, m_myAddress.c_str()); // This should actually be from its listen/host instance
                    registerPacket.port = m_myPort;
                    ENetPacket* packet = enet_packet_create(&registerPacket, sizeof(JNet::BalancedServerRegister), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(m_ENetPeer, 1, packet);
                }
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
