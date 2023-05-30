#include <iostream>
#include <string>

#include <thread>
#include <chrono>

#include "JNetClient.h"
#include "enet/enet.h"

using std::string;
using JNet::Client;

// Demo dependancies, not required by JNet
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <vector>

// Demo GameClient / GameSession data types.
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
    int yourID = -1;
};
struct GSClientPlayerInfo
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerInfo;
    int id = -1;
    char name[16];
    int xPos = 0;
    int yPos = 0;
};
struct GSClientPlayerUpdate
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerUpdate;
    int id = -1;
    int xPos = 0;
    int yPos = 0;
};
struct GSClientPlayerDisconnect
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerDisconnect;
    int id = -1;
};
struct ClientGSJoined
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSJoined;
    char name[16];
    int xPos = 0;
    int yPos = 0;
};
struct ClientGSInitialised
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSInitialised;
};
struct ClientGSPlayerUpdate
{
    JNetDemoGameSessionPacket type = JNetDemoGameSessionPacket::DemoClientToGSUpdate;
    int xPos = 0;
    int yPos = 0;
};

// Demo GameClient configuration.
string playerName = "Player";
std::vector<GSClientPlayerInfo> m_players;
bool m_initialised = false;
int m_playerCount = 0;
int m_playersRecieved = 0;
int m_myId = -1;
ImVec2 playerPosition = { 640, 360 };
ImVec2 playerSize = { 50, 50 };
ImVec2 centrePivot = { 0.5f, 0.5f };

// Demo GameClient callbacks
void PlayerConnectCallBack(_ENetEvent* event);
void PlayerPacketCallBack(_ENetEvent* event);
void PlayerDisconnectCallBack(_ENetEvent* event);

int main()
{
    // Demo Dependcie code, not required by JNet
    GLFWwindow* window;

    if (!glfwInit())
        return -1;
    window = glfwCreateWindow(1280, 720, "JNet Client Demo", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL())
        return -1;
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    string address = "127.0.0.1";
    int port = 6000;
    Client client;
    client.m_ClientConnectCallBack = PlayerConnectCallBack;
    client.m_ClientPacketCallBack = PlayerPacketCallBack;
    client.m_ClientDisconnectCallBack = PlayerDisconnectCallBack;

    ImGuiWindowFlags playerObjectWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs;

    // Loopy
    while (!glfwWindowShouldClose(window))
    {
        // Update JNet client
        client.Update();
        
        // Everything below is the deme implementing it.
        //Update ImGui and the OpenGL/GLFW Window.
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show login prompt
        if (!client.IsConnectedMasterServer() && !client.IsConnectedBalancedServer())
        {
            ImVec2 windowPos = { 640, 360 };
            ImVec2 windowSize = { 300, 130 };
            ImVec2 windowPivot = { 0.5, 0.5 };
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
            ImGui::SetNextWindowSize(windowSize);
            ImGui::Begin("Login", 0 , ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::InputText("Player Name", &playerName);
            ImGui::Spacing();
            ImGui::InputText("Hostname", &address);
            ImGui::InputInt("Port", &port, 0, 0, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsDecimal);

            if (ImGui::Button("Connect"))
            {
                client.Initialise();
                client.SetMasterServer(address, port);
                client.ConnectToMasterServer();
            }
            ImGui::End();
        }

        // Render player and get input.
        if (client.IsConnectedGameSession())
        {
            bool moved = false;
            if (glfwGetKey(window, GLFW_KEY_W))
            {
                playerPosition.y -= 2;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_S))
            {
                playerPosition.y += 2;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_A))
            {
                playerPosition.x -= 2;
                moved = true;
            }
            if (glfwGetKey(window, GLFW_KEY_D))
            {
                playerPosition.x += 2;
                moved = true;
            }

            if (moved)
            {
                // set update packet to server.
                ClientGSPlayerUpdate update;
                update.xPos = (int)playerPosition.x;
                update.yPos = (int)playerPosition.y;

                ENetPacket* updatePacket = enet_packet_create(&update, sizeof(ClientGSPlayerUpdate), ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE);
                enet_peer_send(client.m_ENetGameSessionPeer, 0, updatePacket);
            }


            ImGui::SetNextWindowPos(playerPosition, ImGuiCond_Always, centrePivot);
            ImGui::SetNextWindowSize(playerSize);
            string namePlusID = playerName + "###-1";
            ImGui::Begin(namePlusID.c_str(), 0, playerObjectWindowFlags);
            ImGui::Text("o_o");
            ImGui::End();

            if (m_initialised)
            {
                for (int i = 0; i < m_players.size(); i++)
                {
                    if (m_players[i].id == m_myId)
                        continue;
                    ImVec2 position = { (float)m_players[i].xPos, (float)m_players[i].yPos };
                    ImGui::SetNextWindowPos(position, ImGuiCond_Always, centrePivot);
                    ImGui::SetNextWindowSize(playerSize);
                    string namePlusID = m_players[i].name;
                    namePlusID += "###" + std::to_string(m_players[i].id);
                    ImGui::Begin(namePlusID.c_str(), 0, playerObjectWindowFlags);
                    ImGui::Text("._.");
                    ImGui::End();
                }
            }
        }

        // Show overall status
        ImVec2 windowPos = { 0, 0 };
        ImVec2 windowSize = { 1280, 63 };
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(windowSize);
        ImGui::Begin("Status", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::BeginDisabled();
        // Master Server
        ImGui::Text("Master Server:");
        if (client.IsConnectedMasterServer())
        {
            ImGui::SameLine();
            ImGui::Text("Connected");
        }
        // Balanced Server
        ImGui::Text("Balanced Server:");
        if (client.IsConnectedBalancedServer())
        {
            ImGui::SameLine();
            ImGui::Text("Connected");
            ImGui::SameLine();
            ImGui::Text(" - ");
            ImGui::SameLine();
            ImGui::Text(client.GetBalancedServerName().c_str());
        }
        // Game Session
        ImGui::Text("Game Session:");
        if (client.IsConnectedGameSession())
        {
            ImGui::SameLine();
            ImGui::Text("Connected");
            ImGui::SameLine();
            ImGui::Text(" - ");
            ImGui::SameLine();
            ImGui::Text(client.GetGameSessionName().c_str());
        }
        ImGui::EndDisabled();
        ImGui::End();

        // Close Out
        ImGui::Render();
        if (ImGui::GetDrawData())
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    glfwTerminate();
    return 0;
}

void PlayerConnectCallBack(_ENetEvent* event)
{
    std::cout << "Connected to Game Session." << std::endl;
    // send my own info.
    ClientGSJoined init;
    strcpy_s(init.name, playerName.c_str());
    init.xPos = (int)playerPosition.x;
    init.yPos = (int)playerPosition.y;

    ENetPacket* joinPacket = enet_packet_create(&init, sizeof(ClientGSJoined), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(event->peer, 0, joinPacket);
}

void PlayerPacketCallBack(_ENetEvent* event)
{
    JNetDemoGameSessionPacket* packet = (JNetDemoGameSessionPacket*)event->packet->data;

    switch (*packet)
    {
    case JNetDemoGameSessionPacket::DemoGSToDemoClientInitialise:
    {
        //std::cout << "Received Initialise Packet." << std::endl;
        GSClientInitialise* init = (GSClientInitialise*)event->packet->data;
        m_players.resize(init->playerCount);
        m_playerCount = init->playerCount;
        m_playersRecieved = 0;
        m_myId = init->yourID;
        break;
    }
    case JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerInfo:
    {
        //std::cout << "Received player info packet" << std::endl;
        GSClientPlayerInfo* info = (GSClientPlayerInfo*)event->packet->data;
        if (m_playersRecieved == m_players.size())
            m_players.resize(m_players.size() + 1);

        strcpy_s(m_players[m_playersRecieved].name, info->name);
        m_players[m_playersRecieved].id = info->id;
        m_players[m_playersRecieved].xPos = info->xPos;
        m_players[m_playersRecieved].yPos = info->yPos;
        m_playersRecieved++;
        if (m_playersRecieved == m_playerCount && !m_initialised)
        {
            ClientGSInitialised initDone;

            ENetPacket* initDonePacket = enet_packet_create(&initDone, sizeof(ClientGSInitialised), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event->peer, 0, initDonePacket);
            m_initialised = true;
        }
        break;
    }
    case JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerUpdate:
    {
        if (!m_initialised)
            break;

        //std::cout << "Received player info packet" << std::endl;
        GSClientPlayerUpdate* info = (GSClientPlayerUpdate*)event->packet->data;
        for (int i = 0; i < m_players.size(); i++)
        {
            if (m_players[i].id == info->id)
            {
                m_players[i].xPos = info->xPos;
                m_players[i].yPos = info->yPos;
            }

        }
        break;
    }
    case JNetDemoGameSessionPacket::DemoGSToDemoClientPlayerDisconnect:
    {
        GSClientPlayerDisconnect* info = (GSClientPlayerDisconnect*)event->packet->data;
        for (int i = 0; i < m_players.size(); i++)
        {
            if (m_players[i].id == info->id)
            {
                m_players.erase(m_players.begin() + i);
                m_playerCount--;
                m_playersRecieved--;
                break;
            }
        }
    }
    }
}

void PlayerDisconnectCallBack(_ENetEvent* event)
{
    std::cout << "Client Disconnected" << std::endl;
    m_players.clear();
    m_initialised = false;
    m_playerCount = 0;
    m_playersRecieved = 0;
}
