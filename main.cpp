#include "src/server.hpp"
#include "src/client.hpp"
#include "src/configuration.hpp"

#include <iostream>
#include <cstring> // std::strcmp

int main(int argc, char** argv) {
    std::cout << "Args: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << i << ' ' << argv[i] << std::endl;
    }
    std::cout << std::endl;
    
    // todo 20190419nitzel Forward to and use the settings in the game etc, to replace the #defines in 
    CConfiguration config("settings.ini");
    std::cout << "Config=" << config << std::endl;
    
    // no args: ask user for client/server
    if (argc == 1) {
        // ask user
        char clientOrServer;
        std::cout << "Client(c) or Server(anything else): ";
        std::cin >> clientOrServer;

        switch (clientOrServer) {
        case 'c': 
            return client(config);
        default:
            return server(config);
        }
    }

    // arg -s to start a server, otherwise client
    if (!std::strcmp(argv[1], "-s")) {
        return server(config);
    }
    
    return client(config);
}