#ifndef __CONFIGURATION__
#define __CONFIGURATION__

#include <string>
#include <iostream>
#include "include/inih/INIReader.hpp"

constexpr unsigned int DEFAULT_PORT = 12345;

/// Reads the configuration from an ini file.
/// Has default values for missing entries.
class CConfiguration {
public:
    /// Graphics-section of ini file.
    class CGraphics {
    private:
        static const std::string section;
    public:
        unsigned int width = 640; // pixels
        unsigned int height = 480; // pixels
        bool fullscreen = false;

        CGraphics() {}
        CGraphics(const INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, const CConfiguration::CGraphics& graphics) {
            return os 
                << "[width=" << graphics.width 
                << "; height=" << graphics.height 
                << "; fullscreen=" << graphics.fullscreen 
                << "]";
        }
    };

    /// Graphics-section of ini file.
    class CGame {
    private:
        static const std::string section;
    public:
        unsigned int maxShipsPerPlayer = 1000;

        CGame() {}
        CGame(const INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, const CConfiguration::CGame& game) {
            return os 
                << "[maxShipsPerPlayer=" << game.maxShipsPerPlayer 
                << "]";
        }
    };

    /// Server-section of ini file.
    class CServer {
    private:
        static const std::string section;
    public:
        unsigned int port = DEFAULT_PORT;
        std::string name = "Planet Battle";

        CServer() {}
        CServer(const INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, const CConfiguration::CServer& server) {
            return os 
                << "[name=" << server.name 
                << "; port=" << server.port 
                << "]";
        }
    };

    /// Client-section of ini file.
    class CClient {
    private:
        static const std::string section;
    public:
        unsigned int port = DEFAULT_PORT;
        std::string name = "unknown";
        std::string host = "localhost";

        CClient() {}
        CClient(const INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, const CConfiguration::CClient& client) {
            return os 
                << "[name=" << client.name 
                << "; port=" << client.port 
                << "; host=" << client.host 
                << "]";
        }
    };

private:
public:
    CServer server;
    CClient client;
    CGraphics graphics;
    CGame game;
    //CConfiguration() = delete;
    CConfiguration() {}
    CConfiguration(const std::string iniFilename = "settings.ini");
    friend std::ostream& operator<< (std::ostream& os, const CConfiguration& configuration) {
        return os 
            << "[Server=" << configuration.server 
            << "; Client=" << configuration.client 
            << "; Graphics=" << configuration.graphics 
            << "; Game=" << configuration.game
            << "]";
    }
};

#endif