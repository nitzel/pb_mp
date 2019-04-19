#ifndef __CONFIGURATION__
#define __CONFIGURATION__

#include <string>
#include <iostream>
#include "include/inih/INIReader.hpp"

#define DEFAULT_PORT 12345

/// Reads the configuration from an ini file.
/// Has default values for missing entries.
class CConfiguration {
public:
    /// Server-section of ini file.
    class CServer {
    private:
        static const std::string section;
    public:
        unsigned int port = DEFAULT_PORT;
        std::string name = "Planet Battle";

        CServer();
        CServer(INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, CConfiguration::CServer& server) {
            return os << "[name=" << server.name << "; port=" << server.port << ";]";
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

        CClient();
        CClient(INIReader& iniReader);
        friend std::ostream& operator<< (std::ostream& os, CConfiguration::CClient& client) {
            return os << "[name=" << client.name << "; port=" << client.port << "; host=" << client.host << ";]";
        }
    };

private:
public:
    CServer server;
    CClient client;
    //CConfiguration() = delete;
    CConfiguration(std::string iniFilename = "config.ini");
    friend std::ostream& operator<< (std::ostream& os, CConfiguration& configuration) {
        return os << "[Server=" << configuration.server << "; Client=" << configuration.client << ";]";
    }
};

#endif