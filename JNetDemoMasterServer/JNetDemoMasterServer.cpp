#include <iostream>

#include "JNetMasterServer.h"

using JNet::MasterServer;

int main()
{
    MasterServer m_masterServer;
    m_masterServer.Initialize();
    m_masterServer.Run();
}