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
    if (enet_initialize() != 0)
        std::cout << "Enet failed to initialise." << std::endl;
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
void JNet::Client::SetBalancedServer(string address, unsigned int port, string name)
{
    m_balancedServerAddress = address;
    m_balancedServerPort = port;
    m_balancedServerName = name;
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

void JNet::Client::ConnectToGameSession()
{
    std::cout << "Connecting to Game Session " << m_gameSessionName << std::endl;

    shouldConnectToGameSession = false;

    m_ENetGameSessionClient = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, m_gameSessionAddress.c_str());
    address.port = m_gameSessionPort;

    m_ENetGameSessionPeer = enet_host_connect(m_ENetGameSessionClient, &address, 2, 0);
    if (m_ENetGameSessionPeer != nullptr)
        isConnectedGameSession = true;
    else
        isConnectedGameSession = false;
}

// Updates all JNetClient behaviour. Should be called once per frame in your game loop if you are happy with the basic logic here.
void JNet::Client::Update()
{
    if(isConnectedMasterServer)
        UpdateMasterServer();

    if (isPingingBalancedServers)
        ProcessBalancedServerPinging();

    if (shouldConnectToBalancedServer)
        ConnectToBalancedServer();

    if(isConnectedBalancedServer)
        UpdateBalancedServer();

    if (shouldConnectToGameSession)
        ConnectToGameSession();

    if(isConnectedGameSession)
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
                // TODO something with Auth here. get rid of it? implement it in client??
                strcpy_s(auth.username, "justin");
                strcpy_s(auth.password, "some super password");
                ENetPacket* packet = enet_packet_create(&auth, sizeof(JNet::UserAuth), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(m_ENetMasterServerPeer, 0, packet);
            }
            break;
        case ENET_EVENT_TYPE_RECEIVE:
        {
            //std::cout << "We received a user-defined packet!" << std::endl;
            JNet::JNetPacket* packet = (JNet::JNetPacket*)receivedEvent.packet->data;
            switch (packet->type)
            {
            case JNetPacketType::MSRedirect:
            {
                // Display received server information
                std::cout << "Master Server has redirected us to a Balanced Server!" << std::endl;
                JNet::MasterServerRedirect* redirect = (JNet::MasterServerRedirect*)packet;
                SetBalancedServer(redirect->hostname, redirect->port, redirect->name);
                m_balancedServerReceived = true;
                shouldConnectToBalancedServer = true;
                break;
            }
            case JNetPacketType::MSCheckPingStart:
            {
                JNet::MasterServerCheckPingStart* pingStart = (JNet::MasterServerCheckPingStart*)packet;
                m_balancedServersToPingTotal = pingStart->quantity;
                m_balancedServersToPing = new BalancedServerReference[pingStart->quantity];
                m_balancedServersToPingCurrentReceiving = 0;
                std::cout << "Master Server has requested we ping balanced servers and connect to the fastest one! Total Servers: " << m_balancedServersToPingTotal << std::endl;
                break;
            }
            case JNetPacketType::MSCheckPingServer:
            {
                std::cout << "Master Server has sent us a Balanced Server." << std::endl;
                JNet::MasterServerCheckPingServer* pingServer = (JNet::MasterServerCheckPingServer*)packet;
                m_balancedServersToPing[m_balancedServersToPingCurrentReceiving].name = pingServer->name;
                m_balancedServersToPing[m_balancedServersToPingCurrentReceiving].address = pingServer->hostname;
                m_balancedServersToPing[m_balancedServersToPingCurrentReceiving].port = pingServer->port;
                m_balancedServersToPingCurrentReceiving++;
                break;
            }
            case JNetPacketType::Error:
            {
                // Display received server information
                JNet::ErrorMessage* error = (JNet::ErrorMessage*)packet;
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

    // ping tests go here.
    if (m_balancedServersToPingCurrentReceiving == m_balancedServersToPingTotal)
    {
        m_balancedServersToPingCurrentReceiving = -1;
        isPingingBalancedServers = true;
    }

    if (isPingingBalancedServers)
        ProcessBalancedServerPinging();
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
            JNet::JNetPacket* packet = (JNet::JNetPacket*)receivedEvent.packet->data;
            switch (packet->type)
            {
            case JNetPacketType::BSGameServerInfo:
            {
                JNet::BalancedServerGameSessionInfo* GSInfo = (JNet::BalancedServerGameSessionInfo*)receivedEvent.packet->data;
                m_gameSessionName = GSInfo->name;
                m_gameSessionAddress = GSInfo->address;
                m_gameSessionPort = GSInfo->port;
                m_gameSessionReceived = true;
                shouldConnectToGameSession = true;
                break;
            }
            case JNetPacketType::Error:
            {
                JNet::ErrorMessage* Error = (JNet::ErrorMessage*)receivedEvent.packet->data;
                std::cout << "Received Error from Balanced Server" << std::endl;
                std::cout << Error->message << std::endl;
                break;
            }
            default:
                break;
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have disconnected from the Balanced Server." << std::endl;
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
    ENetEvent receivedEvent; // the variable to place the info in.
    ENetPeer* serverPeer = nullptr;

    // While more information awaits us from the master server...
    while (enet_host_service(m_ENetGameSessionClient, &receivedEvent, 0 /* non-blocking */) > 0)
    {
        // Process
        switch (receivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:

            if (m_ClientConnectCallBack)
                m_ClientConnectCallBack(&receivedEvent);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
        {
            JNet::JNetPacket* packet = (JNet::JNetPacket*)receivedEvent.packet->data;
            switch (packet->type)
            {
            default:
                break;
            }
            if (m_ClientPacketCallBack)
                m_ClientPacketCallBack(&receivedEvent);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have disconnected from the Game Session." << std::endl;
            break;
            if (m_ClientDisconnectCallBack)
                m_ClientDisconnectCallBack(&receivedEvent);
        }
        default:
            std::cout << "some other data received" << std::endl;
            break;
        }
    }

    // Send some bogus packet here for testing.
    /*JNet::ErrorMessage testError;
    ENetPacket* packet = enet_packet_create(&testError, sizeof(JNet::ErrorMessage), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(m_ENetGameSessionPeer, 0, packet);
    std::cout << m_ENetGameSessionPeer->lowestRoundTripTime << std::endl;*/
}

void JNet::Client::ProcessBalancedServerPinging()
{
    if (m_ENetBalancedServerPingClient == nullptr)
    {
        std::cout << "Connecting to server to ping" << std::endl;
        m_ENetBalancedServerPingClient = enet_host_create(nullptr, 1, 2, 0, 0);

        ENetAddress address;
        enet_address_set_host(&address, m_balancedServersToPing[pingServerCurrent].address.c_str());
        address.port = m_balancedServersToPing[pingServerCurrent].port;

        m_ENetBalancedServerPingPeer = enet_host_connect(m_ENetBalancedServerPingClient, &address, 2, 0);
    }

    if (!pingConnected)
    {
        ENetEvent receivedEvent;
        if (enet_host_service(m_ENetBalancedServerPingClient, &receivedEvent, 0) > 0)
        {
            if (receivedEvent.type == ENET_EVENT_TYPE_CONNECT)
            {
                std::cout << "Connected" << std::endl;
                pingConnected = true;
            }

        }
        else return;
    }

    if (pingsSent < pingsToSend)
    {
        // send a ping
        //std::cout << "Sending a Ping" << std::endl;
        pingsSent++;
        JNet::Ping ping;
        ENetPacket* pingPacket = enet_packet_create(&ping, sizeof(JNet::Ping), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_ENetBalancedServerPingPeer, 0, pingPacket);
        //enet_peer_ping(m_ENetBalancedServerPingPeer);
    }
    if (pingsReceived < pingsToSend)
    {
        // uhh dunno why i wrote this. check for ping pax?
        //std::cout << "Checking for Pongs" << std::endl;
        ENetEvent receivedEvent;
        while (enet_host_service(m_ENetBalancedServerPingClient, &receivedEvent, 0 /* non-blocking */) > 0)
        {
            // Process
            switch (receivedEvent.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                JNet::Ping* packet = (JNet::Ping*)receivedEvent.packet->data;
                if (packet->type == JNetPacketType::Ping)
                {
                    pingsReceived++;
                    //std::cout << "Pong!" << std::endl;
                }
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                break;
            }
            default:
                break;
            }
        }
    }
    else return;

    if (pingsSent == pingsToSend && pingsReceived == pingsToSend)
    {
        std::cout << "All sent pings confirmed received! Evaluating" << std::endl;
        // record least response time, check if its radder than previous, close connection, increment test index
        if (m_ENetBalancedServerPingPeer->lowestRoundTripTime < leastResponseTime)
        {
            std::cout << "This is the new fast server!" << std::endl;
            leastResponseTime = m_ENetBalancedServerPingPeer->lowestRoundTripTime;
            leastResponseIndex = pingServerCurrent;
        }
        else
            std::cout << "This server is slower" << std::endl;

        pingServerCurrent++;
        enet_host_destroy(m_ENetBalancedServerPingClient);
        m_ENetBalancedServerPingClient = nullptr;
        m_ENetBalancedServerPingPeer = nullptr;
        pingsSent = 0;
        pingsReceived = 0;

        pingConnected = false;
    }
    else return;

    if (pingServerCurrent >= m_balancedServersToPingTotal)
    {
        std::cout << "Connecting to " << m_balancedServersToPing[leastResponseIndex].name << " with lowest ping of: " << leastResponseTime << std::endl;
        SetBalancedServer(m_balancedServersToPing[leastResponseIndex].address, m_balancedServersToPing[leastResponseIndex].port, m_balancedServersToPing[leastResponseIndex].name);
        shouldConnectToBalancedServer = true;
        isPingingBalancedServers = false;
        pingServerCurrent = 0;
        leastResponseTime = INT_MAX;
    }
    else
        std::cout << "Checking next server" << std::endl;
}
