#include <iostream>
#include "JNetMasterServer.h"
#include <thread>
#include <chrono>

using JNet::MasterServer;

int main()
{
    MasterServer m_masterServer;
    m_masterServer.Initialize();
    // TODO Set mode from here.
    while (true)
    {
        m_masterServer.Process();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}