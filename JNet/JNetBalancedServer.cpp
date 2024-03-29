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
    if (enet_initialize() != 0)
        std::cout << "Enet failed to initialise." << std::endl;
}

void JNet::BalancedServer::SetMasterServer(string address, unsigned int port)
{
    m_masterServerAddress = address;
    m_masterServerPort = port;
}

void JNet::BalancedServer::SetMyConnectionInfo(string myName, string myAddress, unsigned int myPort, unsigned int myPortGS)
{
    m_myName = myName;
    m_myAddress = myAddress;
    m_myPort = myPort;
    m_myPortGS = myPortGS;
}

void JNet::BalancedServer::ConnectToMasterServer()
{
    m_ENetMasterServerClient = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, m_masterServerAddress.c_str());
    address.port = m_masterServerPort;

    
    m_ENetMasterServerPeer = enet_host_connect(m_ENetMasterServerClient, &address, 2, 0);
}

void JNet::BalancedServer::CheckInWithMasterServer()
{
    JNet::BalancedServerUpdate update;
    update.playerCount = m_playerCount;
    update.playerCapacity = (int)m_ENetBalancedServerClient->peerCount;
    update.sessionCount = m_sessionCount;
    update.open = true;
    ENetPacket* packet = enet_packet_create(&update, sizeof(JNet::BalancedServerUpdate), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(m_ENetMasterServerPeer, 1, packet);

    m_MasterServerLastCheckInTime = std::chrono::system_clock::now();
}

void JNet::BalancedServer::OpenForConnections()
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = m_myPort;

    m_ENetBalancedServerClient = enet_host_create(&address, 32, 2, 0, 0);

    ENetAddress addressGS;
    addressGS.host = ENET_HOST_ANY;
    addressGS.port = m_myPortGS;

    m_ENetGameSessionClient = enet_host_create(&addressGS, 32, 2, 0, 0);

    CheckInWithMasterServer();
}

void JNet::BalancedServer::AddCountryCode(string code)
{
    m_countryCodes.push_back(code);
}

void JNet::BalancedServer::SendCountryCodesToMasterServer()
{
    for (int i = 0; i < m_countryCodes.size(); i++)
    {
        JNet::BalancedServerAddCountryCode code;
        strcpy_s(code.name, m_countryCodes[i].c_str());

        ENetPacket* packet = enet_packet_create(&code, sizeof(JNet::BalancedServerAddCountryCode), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_ENetMasterServerPeer, 0, packet);
    }
}

void JNet::BalancedServer::Update()
{
    UpdateMasterServer();
    if(m_ENetGameSessionClient != nullptr)
        UpdateGameSessions();
    if(m_ENetBalancedServerClient != nullptr)
        UpdateClients();
}

void JNet::BalancedServer::UpdateMasterServer()
{
    ENetEvent MSreceivedEvent; // the variable to place the info in.

    // Communications from Master Server
    while (enet_host_service(m_ENetMasterServerClient, &MSreceivedEvent, 0) > 0)
    {
        switch (MSreceivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "We have confirmed connection with the Master Server. Registering self." << std::endl;
            JNet::BalancedServerRegister registerPacket;
            strcpy_s(registerPacket.name, m_myName.c_str());
            strcpy_s(registerPacket.hostname, m_myAddress.c_str()); // This should actually be from its listen/host instance
            registerPacket.port = m_myPort;
            registerPacket.playerCount = m_playerCount;
            registerPacket.playerCapacity = m_playerCapacity;
            registerPacket.sessionCount = (int)m_connectedGameSessions.size();
            registerPacket.open = m_openForConnections;
            ENetPacket* packet = enet_packet_create(&registerPacket, sizeof(JNet::BalancedServerRegister), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(m_ENetMasterServerPeer, 1, packet);
            m_connectedToMasterServer = true;

            SendCountryCodesToMasterServer();

            if (m_MasterServerConnectCallBack)
                m_MasterServerConnectCallBack(&MSreceivedEvent);

            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::cout << "We received a user-defined packet from the Master Server" << std::endl;

            if (m_MasterServerPacketCallBack)
                m_MasterServerPacketCallBack(&MSreceivedEvent);

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have disconnected from the Master Server." << std::endl;

            if (m_MasterServerDisconnectCallBack)
                m_MasterServerDisconnectCallBack(&MSreceivedEvent);

            break;
        }
        default:
            std::cout << "Some other data received from the Master Server" << std::endl;
            break;
        }
    }

    // Update internal state regarding master server.
    if (m_connectedToMasterServer && !m_openForConnections)
    {
        OpenForConnections();
        m_openForConnections = true;
    }

    // Send the master server an update on our current status.
    if (m_connectedToMasterServer && std::chrono::system_clock::now() - m_MasterServerLastCheckInTime > std::chrono::seconds(m_MasterServerCheckInSeconds))
        CheckInWithMasterServer();
}

void JNet::BalancedServer::UpdateGameSessions()
{
    // communications from connected clients
    ENetEvent GSreceivedEvent; // the variable to place the info in.
    while (enet_host_service(m_ENetGameSessionClient, &GSreceivedEvent, 0) > 0)
    {
        switch (GSreceivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << "We have had a GameSession connect." << std::endl;
            m_sessionCount++;

            if (m_GameSessionConnectCallBack)
                m_GameSessionConnectCallBack(&GSreceivedEvent);

            break;
        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::cout << "We received a user-defined packet from a GameSession." << std::endl;
            JNetPacket* packet = (JNetPacket*)GSreceivedEvent.packet->data;
            switch (packet->type)
            {
            case JNetPacketType::GSRegister:
            {
                GameSessionRegister* GSRegister = (GameSessionRegister*)GSreceivedEvent.packet->data;
                ConnectedGameSession gameSession;
                gameSession.name = GSRegister->name;
                gameSession.address = GSRegister->hostname;
                gameSession.port = GSRegister->port;
                gameSession.peer = GSreceivedEvent.peer;
                m_connectedGameSessions.push_back(gameSession);
                std::cout << "New Game Session registered: \"" + gameSession.name + "\"" << std::endl;
                break;
            }
            default:
                break;
            }

            if (m_GameSessionPacketCallBack)
                m_GameSessionPacketCallBack(&GSreceivedEvent);

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have had a GameSession disconnect." << std::endl;
            m_sessionCount--;
            for (int i = 0; i < m_connectedGameSessions.size(); i++)
            {
                if (m_connectedGameSessions[i].peer == GSreceivedEvent.peer)
                {
                    m_connectedGameSessions.erase(m_connectedGameSessions.begin() + i);
                    break;
                }
            }


            if (m_GameSessionDisconnectCallBack)
                m_GameSessionDisconnectCallBack(&GSreceivedEvent);

            break;
        }
        default:
            std::cout << "We received some other data from a GameSession." << std::endl;
            break;
        }
    }
}

void JNet::BalancedServer::UpdateClients()
{
    // communications from connected clients
    ENetEvent BSreceivedEvent; // the variable to place the info in.
    while (enet_host_service(m_ENetBalancedServerClient, &BSreceivedEvent, 0) > 0)
    {
        switch (BSreceivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "We have had a Client connect." << std::endl;
            m_playerCount++;

            // To automatically connect the user to the first game session.
            /*if (m_connectedGameSessions.size() > 0)
            {
                JNet::BalancedServerConnectToGameSession GSInfo;
                strcpy_s(GSInfo.name, m_connectedGameSessions[0].name.c_str());
                strcpy_s(GSInfo.address, m_connectedGameSessions[0].address.c_str());
                GSInfo.port = m_connectedGameSessions[0].port;
                ENetPacket* packet = enet_packet_create(&GSInfo, sizeof(JNet::BalancedServerConnectToGameSession), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(BSreceivedEvent.peer, 1, packet);
            }
            else
            {
                JNet::ErrorMessage Error;
                strcpy_s(Error.message, "No Game Sessions to connect to");
                ENetPacket* packet = enet_packet_create(&Error, sizeof(JNet::ErrorMessage), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(BSreceivedEvent.peer, 1, packet);
            }*/

            if (m_ClientConnectCallBack)
                m_ClientConnectCallBack(&BSreceivedEvent);

            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::cout << "We received a user-defined packet from a Client." << std::endl;
            JNetPacket* packet = (JNetPacket*)BSreceivedEvent.packet->data;
            switch (packet->type)
            {
            case JNetPacketType::Ping:
            {
                // dont bother converting

                ENetPacket* pingPacket = enet_packet_create(packet, sizeof(Ping), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(BSreceivedEvent.peer,0,pingPacket);
                break;
            }
            case JNetPacketType::ClientRequestForAllGS:
            {
                for (auto const session : m_connectedGameSessions)
                {
                    BalancedServerGameSessionInfo info;
                    strcpy_s(info.name, session.name.c_str());
                    strcpy_s(info.address, session.address.c_str());
                    info.players = 0; // TODO fix.
                    info.port = session.port;
                    ENetPacket* gsPacket = enet_packet_create(&info, sizeof(BalancedServerGameSessionInfo), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(BSreceivedEvent.peer, 0, gsPacket);
                }
                break;
            }
            }

            if (m_ClientPacketCallBack)
                m_ClientPacketCallBack(&BSreceivedEvent);

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "We have had a Client disconnect." << std::endl;
            m_playerCount--;

            if (m_ClientDisconnectCallBack)
                m_ClientDisconnectCallBack(&BSreceivedEvent);

            break;
        }
        default:
            std::cout << "We received some other data from a Client." << std::endl;
            break;
        }
    }
}