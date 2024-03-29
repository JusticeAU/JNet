#include <iostream>
#include "JNetMasterServer.h"
#include <thread>
#include <chrono>

using JNet::MasterServer;

int main(int argc, char* argv[]) // note the starting arguments here
{
    // Create our server instance
    MasterServer m_masterServer;
    if(argc > 1)
        m_masterServer.SetClientConnectionPort(atoi(argv[1]));
    if(argc > 2)
        m_masterServer.SetBSConnectionPort(atoi(argv[2]));

    // Set the balance mode.
    m_masterServer.SetBalanceMode(JNet::MasterServer::BalanceMode::LeastConnection);

    // This must be called if the balance mode is set to geo location.
    //m_masterServer.LoadGeoIPDatabase("asn-country-ipv4.csv"); // https://github.com/sapics/ip-location-db

    // This initialises the ENet Hosts for clients to connect to, and balanced servers to connect to.
    m_masterServer.Start();

    while (true)
    {
        m_masterServer.Process();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}