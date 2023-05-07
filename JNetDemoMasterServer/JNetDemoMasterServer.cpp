#include <iostream>

#include <thread>
#include <chrono>

#include "JNetMasterServer.h"

using JNet::MasterServer;

int main()
{
    MasterServer m_masterServer;
    m_masterServer.Initialize();
    m_masterServer.Run();
}