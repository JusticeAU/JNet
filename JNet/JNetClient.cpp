#include "JNetClient.h"
#include "enet/enet.h"
#include "JNetPackets.h"

#include <iostream>

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
	m_masterServerAddress = address;
	m_masterServerPort = port;
}

void JNet::Client::ConnectToMasterServer()
{
	m_ENetClient = enet_host_create(nullptr, 1, 2, 0, 0);
	
    ENetAddress address;
    enet_address_set_host(&address, m_masterServerAddress.c_str());
    address.port = m_masterServerPort;

	m_ENetPeer = enet_host_connect(m_ENetClient, &address, 2, 0);
}

void JNet::Client::Run()
{
    while (true) // client loop
    {
        ENetEvent receivedEvent; // the variable to place the info in.
        ENetPeer* serverPeer = nullptr;

        // While more information awats us...
        while (enet_host_service(m_ENetClient, &receivedEvent, 100) > 0)
        {
            // Process
            switch (receivedEvent.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "We have confirmed connection with the server. Sending Auth" << std::endl;
                {
                    JNet::UserAuth auth;
                    strcpy_s(auth.username, "justin");
                    strcpy_s(auth.password, "some super password");
                    ENetPacket* packet = enet_packet_create(&auth, sizeof(JNet::UserAuth), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(m_ENetPeer, 0, packet);
                }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::cout << "We received a user-defined packet!" << std::endl;
                JNet::JNetPacket* packet = (JNet::JNetPacket*)receivedEvent.packet->data;
                switch (packet->type)
                {
                case JNetPacketType::MSRedirect:
                {
                    // Display received server information
                    JNet::MasterServerRedirect* redirect = (JNet::MasterServerRedirect*)packet;
                    std::cout << redirect->name << std::endl;
                    std::cout << redirect->hostname << std::endl;
                    std::cout << redirect->port << std::endl;
                    break;
                }
                case JNetPacketType::MSError:
                {
                    // Display received server information
                    JNet::MasterServerErrorMessage* error = (JNet::MasterServerErrorMessage*)packet;
                    std::cout << error->message << std::endl;
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "We have disconnected from the server." << std::endl;
                break;
            }
            default:
                std::cout << "some other data received" << std::endl;
                break;
            }
        }
    }
}
