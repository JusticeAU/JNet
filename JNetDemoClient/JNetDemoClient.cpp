#include <iostream>
#include <string>

#include <thread>
#include <chrono>

#include "JNetClient.h"

using std::string;
using JNet::Client;

int main()
{
    // Get IP of master server
    std::cout << "Enter the IP Address or Hostname of the Master Server to connect to:\n";
    string address;
    std::cin >> address;

    Client client;
    client.Initialise();
    client.SetMasterServer(address, 6000);
    client.Connect();

    // Auth, get redirected to Balanced server

    // Auth, stay connected to Balanced Server but also connecto Game Session

    // Loop.
}
