#include "configuration.hpp"

const std::string CConfiguration::CServer::section = "Server";
const std::string CConfiguration::CClient::section = "Client";
const std::string CConfiguration::CGraphics::section = "Graphics";
const std::string CConfiguration::CGame::section = "Game";

///////////////////////////////////////////////////////////////////////////////////////////////////
// Server

CConfiguration::CServer::CServer(const INIReader& iniReader) :
    port(iniReader.GetInteger(section, "port", port)),
    name(iniReader.GetString(section, "name", name)) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Client

CConfiguration::CClient::CClient(const INIReader& iniReader) :
    port(iniReader.GetInteger(section, "port", port)),
    name(iniReader.GetString(section, "name", name)),
    host(iniReader.GetString(section, "host", host)) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics

CConfiguration::CGraphics::CGraphics(const INIReader& iniReader) :
    width(iniReader.GetInteger(section, "width", width)),
    height(iniReader.GetInteger(section, "height", height)),
    fullscreen(iniReader.GetBoolean(section, "fullscreen", fullscreen)) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Game

CConfiguration::CGame::CGame(const INIReader& iniReader) :
    maxShipsPerPlayer(iniReader.GetInteger(section, "maxShipsPerPlayer", maxShipsPerPlayer)) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration

CConfiguration::CConfiguration(const std::string iniFilename) {
    INIReader iniReader(iniFilename);
    server = CConfiguration::CServer(iniReader);
    client = CConfiguration::CClient(iniReader);
    graphics = CConfiguration::CGraphics(iniReader);
    game = CConfiguration::CGame(iniReader);
}
