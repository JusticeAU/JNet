#include <iostream>
#include <string>
#include "JNetBalancedServer.h"


int main(int argc, char* argv[]) // note the starting arguments here
{
    // Balanced server should take the IP address and port arguments provided to it on launch, initialise itself and connect to the master server under this balanced server contexts and 'check in'
    string address;
    int port;
    string myName = "Unnamed Balanced Server";
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
        std::cout << "Enter the IP Address or Hostname of the Master Server to connect to:\n";
        std::cin >> address;
        port = 6000;
    }

    // Initialise JNet
    JNet::BalancedServer server;
    server.Initialise();
    server.SetMasterServer(address, port);
    server.SetMyConnectionInfo(myName, myAddress, myPort);
    server.ConnectToMasterServer();

    while (true)
    {
        server.Update();   

    }
}