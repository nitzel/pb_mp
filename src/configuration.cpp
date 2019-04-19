#include "configuration.hpp"

const std::string CConfiguration::CServer::section = "Server";
const std::string CConfiguration::CClient::section = "Client";

CConfiguration::CServer::CServer(INIReader& iniReader) :
    port(iniReader.GetInteger(CServer::section, "port", port)),
    name(iniReader.GetString(CServer::section, "name", name)) {
}

CConfiguration::CServer::CServer() {
}

CConfiguration::CClient::CClient(INIReader& iniReader) :
    port(iniReader.GetInteger(CClient::section, "port", port)),
    name(iniReader.GetString(CClient::section, "name", name)),  
    host(iniReader.GetString(CClient::section, "host", host)) {
}

CConfiguration::CClient::CClient() {
}


CConfiguration::CConfiguration(std::string iniFilename) {
    INIReader iniReader(iniFilename);
    server = CConfiguration::CServer(iniReader);
    client = CConfiguration::CClient(iniReader);
}
