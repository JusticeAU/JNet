#include <iostream>
#include <string>
#include "JNetGameSession.h"

#include <thread>
#include <chrono>

// Demo GameSession dependencies
#include <vector>
#include "enet/enet.h"

// Demo Game/GameSession data types.
enum class JNetDemoGameSessionPacket
{
    DemoGSToDemoClientInitialise = 50,
    DemoGSToDemoClientPlayerInfo,
    DemoGSToDemoClientPlayerUpdate,
    DemoGSToDemoClientPlayerDisconnect,
    DemoClientToGSJoined,
    DemoClientToGSInitialised,
    DemoClientToGSUpdate,
};

struct GSClientInitialise
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientInitialise;
    int playerCount;
    int yourID;
};
struct GSClientPlayerInfo
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerInfo;
    int id;
    char name[16];
    int xPos;
    int yPos;
};
struct GSClientPlayerUpdate
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerUpdate;
    int id;
    int xPos;
    int yPos;
};
struct GSClientPlayerDisconnect
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerDisconnect;
    int id;
};
struct ClientGSJoined
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSJoined;
    char name[16];
    int xPos;
    int yPos;
};
struct ClientGSInitialised
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSInitialised;
};
struct ClientGSPlayerUpdate
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSUpdate;
    int xPos;
    int yPos;
};

struct PlayerInfo
{
    _ENetPeer* peer = nullptr;
    GSClientPlayerInfo info;
    bool initialised = false;
};

// Demo GameSession Configuration
std::vector<PlayerInfo> players;
int playerIDs = 0;
JNet::GameSession gs;

// Demo GameSession Callbacks to register with JNet client.
void PlayerConnectCallBack(_ENetEvent* event);
void PlayerPacketCallBack(_ENetEvent* event);
void PlayerDisconnectCallBack(_ENetEvent* event);

int main(int argc, char* argv[]) // note the starting arguments here
{
    // Balanced server should take the IP address and port arguments provided to it on launch, initialise itself and connect to the master server under this balanced server contexts and 'check in'
    string address;
    int port;
    string myName = "Unnamed Game Session";
    string myAddress = "127.0.0.1";
    int myPort = 6050;

    if (argc >= 3)
    {
        address = argv[1];
        port = atoi(argv[2]);

        // if details for self were provided.
        if (argc == 6)
        {
            myName = argv[3];
            myAddress = argv[4];
            myPort = atoi(argv[5]);
        }
    }
    else
    {
        std::cout << "Enter the IP Address or Hostname of the Balanced Server to connect to:\n";
        std::cin >> address;
        port = 6000;
    }

    // Initialise JNet
    // Set callbacks.
    gs.m_ClientConnectCallBack = PlayerConnectCallBack;
    gs.m_ClientPacketCallBack = PlayerPacketCallBack;
    gs.m_ClientDisconnectCallBack = PlayerDisconnectCallBack;
    
    // run Initialisation and start connections.
    gs.Initialise();
    gs.SetBalancedServer(address, port);
    gs.SetMyConnectionInfo(myName, myAddress, myPort);
    gs.ConnectToBalancedServer();
    gs.StartMyHost();
    while (true)
    {
        gs.Process();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void PlayerConnectCallBack(_ENetEvent* event)
{
    std::cout << "Player Connected." << std::endl;

}

void PlayerPacketCallBack(_ENetEvent* event)
{
    JNetDemoGameSessionPacket* packet = (JNetDemoGameSessionPacket*)event->packet->data;

    switch (*packet)
    {
    case JNetDemoGameSessionPacket::DemoClientToGSJoined:
    {
        ClientGSJoined* joinInfo = (ClientGSJoined*)event->packet->data;

        PlayerInfo pi;
        pi.peer = event->peer;
        std::cout << playerIDs << std::endl;
        pi.info.id = playerIDs++;
        strcpy_s(pi.info.name, joinInfo->name);
        pi.info.xPos = joinInfo->xPos;
        pi.info.yPos = joinInfo->yPos;
        players.push_back(pi);

        // Send Init Packet
        GSClientInitialise init;
        init.yourID = pi.info.id;
        init.playerCount = players.size();
        ENetPacket* initPacket = enet_packet_create(&init, sizeof(GSClientInitialise), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event->peer, 0, initPacket);

        for (int i = 0; i < players.size(); i++)
        {
            GSClientPlayerInfo info;
            info.id = players[i].info.id;
            strcpy_s(info.name, players[i].info.name);
            info.xPos = players[i].info.xPos;
            info.yPos = players[i].info.yPos;
            ENetPacket* playerInfoPacket = enet_packet_create(&info, sizeof(GSClientPlayerInfo), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event->peer, 0, playerInfoPacket);
        }
        break;
    }
    case JNetDemoGameSessionPacket::DemoClientToGSInitialised:
    {
        std::cout << "Client Initialised" << std::endl;
        int thisPeerID = -1;
        int thisPeerIndex = -1;
        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].peer == event->peer)
            {
                thisPeerID = players[i].info.id;
                thisPeerIndex = i;
                break;
            }
        }

        // prepare a packet to send to all existing clients
        GSClientPlayerInfo info;
        info.id = thisPeerID;
        strcpy_s(info.name, players[thisPeerIndex].info.name);
        info.xPos = players[thisPeerIndex].info.xPos;
        info.yPos = players[thisPeerIndex].info.yPos;
        ENetPacket* playerInfoPacket = enet_packet_create(&info, sizeof(GSClientPlayerInfo), ENET_PACKET_FLAG_RELIABLE);

        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].initialised)
                enet_peer_send(players[i].peer, 0, playerInfoPacket);
        }

        players[thisPeerIndex].initialised = true;

        break;
    }
    case JNetDemoGameSessionPacket::DemoClientToGSUpdate:
    {
        ClientGSPlayerUpdate* update = (ClientGSPlayerUpdate*)event->packet->data;

        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].peer == event->peer)
            {
                players[i].info.xPos = update->xPos;
                players[i].info.yPos = update->yPos;

                GSClientPlayerUpdate clientUpdate;
                clientUpdate.id = players[i].info.id;
                clientUpdate.xPos = update->xPos;
                clientUpdate.yPos = update->yPos;

                ENetPacket* clientUpdatePacket = enet_packet_create(&clientUpdate, sizeof(GSClientPlayerUpdate), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
                enet_host_broadcast(gs.m_ENetGameSessionClient, 0, clientUpdatePacket);
                break;
            }
        }
        break;
    }
    }
}

void PlayerDisconnectCallBack(_ENetEvent* event)
{
    std::cout << "Client Disconnected from Server Call Back" << std::endl;
    // Get the player ID and broadcast to all remaining players that they disconnected.
    int id = -1;
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].peer == event->peer)
        {
            id = players[i].info.id;
            players.erase(players.begin() + i);
            break;
        }
    }

    if (id != -1)
    {
        // broadcast disconnection to remaining players.
        GSClientPlayerDisconnect clientDisconnection;
        clientDisconnection.id = id;

        ENetPacket* clientDisconnectionPacket = enet_packet_create(&clientDisconnection, sizeof(GSClientPlayerDisconnect), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
        enet_host_broadcast(gs.m_ENetGameSessionClient, 0, clientDisconnectionPacket);
    }
}