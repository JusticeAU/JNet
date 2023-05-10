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

// Initialise ENet back end.
void JNet::Client::Initialise()
{
    if (enet_initialize() == 0)
        std::cout << "ENet Initialized!" << std::endl;
    else
        std::cout << "Failure! ENet failed to initialize" << std::endl;
}

// Set the address and port that will be used when ConnectToMasterServer is called.
void JNet::Client::SetMasterServer(string address, unsigned int port)
{
	m_masterServerAddress = address;
	m_masterServerPort = port;
}

// Attempts to connect to the master server using the Address and Port set with SetMasterServer
void JNet::Client::ConnectToMasterServer()
{
	m_ENetMasterServerClient = enet_host_create(nullptr, 1, 2, 0, 0);
	
    ENetAddress address;
    enet_address_set_host(&address, m_masterServerAddress.c_str());
    address.port = m_masterServerPort;

	m_ENetMasterServerPeer = enet_host_connect(m_ENetMasterServerClient, &address, 2, 0);
    if (m_ENetMasterServerPeer != nullptr)
        isConnectedMasterServer = true;
    else
        isConnectedMasterServer = false;
}

// Set the address and port that wil lbe used when ConnectToBalanced server is called. Usually called internally when the information is received from the Master Server.
void JNet::Client::SetBalancedrServer(string address, unsigned int port)
{
    m_balancedServerAddress = address;
    m_balancedServerPort = port;
}

// Attempts to connect to the Balanced Server using the Address and Port set with SetMasterServer
void JNet::Client::ConnectToBalancedServer()
{
    std::cout << "Connecting to Balanced Server " << m_balancedServerName << std::endl;

    shouldConnectToBalancedServer = false;

    m_ENetBalancedServerClient = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, m_balancedServerAddress.c_str());
    address.port = m_balancedServerPort;

    m_ENetBalancedServerPeer = enet_host_connect(m_ENetBalancedServerClient, &address, 2, 0);
    if (m_ENetBalancedServerPeer != nullptr)
        isConnectedBalancedServer = true;
    else
        isConnectedBalancedServer = false;
}

// Updates all JNetClient behaviour. Should be called once per frame in your game loop if you are happy with the basic logic here.
void JNet::Client::Update()
{
    if(isConnectedMasterServer)
        UpdateMasterServer();
    if (shouldConnectToBalancedServer)
        ConnectToBalancedServer();

    if(isConnectedBalancedServer)
        UpdateBalancedServer();
    if(isConectedGameSession)
        UpdateGameSession();
}

void JNet::Client::UpdateMasterServer()
{
    ENetEvent receivedEvent; // the variable to place the info in.
    ENetPeer* serverPeer = nullptr;

    // While more information awaits us from the master server...
    while (enet_host_service(m_ENetMasterServerClient, &receivedEvent, 0 /* non-blocking */) > 0)
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
                enet_peer_send(m_ENetMasterServerPeer, 0, packet);
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
                m_balancedServerName = redirect->name;
                m_balancedServerAddress = redirect->hostname;
                m_balancedServerPort = redirect->port;
                m_balancedServerReceived = true;
                shouldConnectToBalancedServer = true;
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
            std::cout << "We have disconnected from the Master Server." << std::endl;
            m_shouldCloseMasterServerHost = true;

            break;
        }
        default:
            std::cout << "some other data received" << std::endl;
            break;
        }
    }

    if (m_ENetMasterServerClient != nullptr && m_shouldCloseMasterServerHost)
    {
        enet_host_destroy(m_ENetMasterServerClient);
        m_ENetMasterServerClient = nullptr;
        isConnectedMasterServer = false;
    }
}

void JNet::Client::UpdateBalancedServer()
{
    ENetEvent receivedEvent; // the variable to place the info in.
    ENetPeer* serverPeer = nullptr;

    // While more information awaits us from the master server...
    while (enet_host_service(m_ENetBalancedServerClient, &receivedEvent, 0 /* non-blocking */) > 0)
    {
        // Process
        switch (receivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            break;
        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::cout << "We received a user-defined packet!" << std::endl;
            JNet::JNetPacket* packet = (JNet::JNetPacket*)receivedEvent.packet->data;
            switch (packet->type)
            {
            default:
                break;
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have disconnected from the Master Server." << std::endl;
            break;
        }
        default:
            std::cout << "some other data received" << std::endl;
            break;
        }
    }
}

void JNet::Client::UpdateGameSession()
{
}
