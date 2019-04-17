#include "src/server.hpp"
#include "src/client.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Args: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << i << ' ' << argv[i] << std::endl;
    }
    std::cout << std::endl;

    if (argc == 1) {
        // ask user
        char clientOrServer;
        std::cout << "Client(c) or Server(anything else): ";
        std::cin >> clientOrServer;

        switch (clientOrServer) {
        case 'c': 
            return client("localhost");
        default:
            return server(1000);
        }
    }


    if (!strcmp(argv[1], "-s")) {
        int maxShips = argc == 3 ? std::stoi(argv[2]) : 1000;
        return server(maxShips);
    }
    
    std::string hostname = "localhost";
    if (argc == 2 && strcmp(argv[1], "-c"))
        hostname = std::string(argv[1]);
    else if (argc == 3)
        hostname = std::string(argv[2]);
    return client(hostname);
}