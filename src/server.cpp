#include "server.hpp"

struct ClientData {
    Party party;
    ENetPeer* peer;
    bool isReady;
};
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
bool allClientsReady(ClientData* clientData, const size_t size);
void broadcastIfReadystateChanged(ENetHost* host, const bool formerReadyState, const bool newReadyState);

int server(size_t shipAmount) {
    std::cout << "Server" << ", maximum number of ships is " << shipAmount << std::endl;
    //freopen("stderr_client.txt", "w", stderr);
    
    //////////
    // ENET
    /////////
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetHost* host;
    ENetAddress address;
    ENetEvent event;

    // server
    printf("Server\n");
    address.host = ENET_HOST_ANY;
    address.port = 12345;
    host = enet_host_create(&address, 2, 3, 0, 0); // 2 connections, 3 channels

    if (host == NULL) {
        printf("An error occured while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }
    enet_host_bandwidth_throttle(host);

    ////////////////
    // VARS
    ////////////////
    ClientData clientData[PN];
    for (size_t party = 0; party < PN; party++) {
        clientData[party].party = (Party)party;
        clientData[party].peer = nullptr;
        clientData[party].isReady = false;
    }

    double time = glfwGetTime();
    double dt = 0, vdt = 0; // virtuel dt, added to dt on data-arrival
    double fps = 0;
    bool paused = true;

    mouseR = { 0,0 };
    mouseV = { 0,0 };
    screen = { 640,480 }; // {640, 480};//
    view = { 0,0 };
    ////////////////
    // GAME VARS
    ////////////////

    Game game(vec2{ 2000,2000 }, shipAmount, 6);

    ///////////////////////////////
    // init GLFW
    /////////////////////////////////s
    initGlfw("PB-MP-SERVER", (int)screen.x, (int)screen.y);
    initGfx();
    // add input listeners
    glfwSetCursorPosCallback(info.window, cursor_pos_callback);


    double timeToBroadcast = 0;
    // GAME LOOP
    while (!glfwWindowShouldClose(info.window)) {
        // update timer
        dt = glfwGetTime() - time;
        time = glfwGetTime();
        fps = (fps * 500 + 10 / dt) / 510;
        // move view
        {
            float dx = 0, dy = 0;
            if (mouseR.x < 80) dx = mouseR.x - 80;
            if (mouseR.y < 80) dy = mouseR.y - 80;
            if (mouseR.x > screen.w - 80) dx = mouseR.x - (screen.w - 80);
            if (mouseR.y > screen.h - 80) dy = mouseR.y - (screen.h - 80);
            view.x += dx / 80.f * 20;
            view.y += dy / 80.f * 20;
            if (view.x < 0) view.x = 0;
            if (view.y < 0) view.y = 0;
            if (view.x > game.mMap.w - screen.w) view.x = game.mMap.w - screen.w;
            if (view.y > game.mMap.h - screen.h) view.y = game.mMap.h - screen.h;
        }
        // clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW); // reset the matrix
        glLoadIdentity();

        // process game content
        if (!paused) {
            game.update(dt + vdt);
            game.shootAndCollide();
            vdt = 0;
        }

        // draw gamecontent
        drawPlanets(game.mPlanets, -view.x, -view.y);
        drawShips(game.mShips, -view.x, -view.y);
        drawShots(game.mShots, -view.x, -view.y);
        drawTree(game.mTree, game.treeW, game.treeH, -view.x, -view.y);

        char s[100];
        snprintf(s, 100, "FPS=%4.0f t=%.1fs Money A=%5i B=%5i MR%i/%i MV%i/%i View%i/%i", fps, time, (int)game.mMoney[PA], (int)game.mMoney[PB], (int)mouseR.x, (int)mouseR.y, (int)mouseV.x, (int)mouseV.y, (int)view.x, (int)view.y);
        glColor3ub(255, 255, 255);
        drawString(s, strlen(s), 10, 10);

        glfwSwapBuffers(info.window);
        glfwPollEvents();

        timeToBroadcast -= dt;
        if (timeToBroadcast < 0)
        {
            timeToBroadcast = 0.1; /// 2x per sec
            size_t size;
            void* d = game.packUpdateData(size, glfwGetTime());
            ENetPacket* packet = enet_packet_create(d, size, 0, PTYPE_UPDATE);// ENET_PACKET_FLAG_RELIABLE

            enet_host_broadcast(host, 1, packet);
            enet_host_flush(host);
            free(d);
            game.clearChanged();
        }

        while (enet_host_service(host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                Party assignedParty = PN;
                for (size_t party = 0; party < PN; party++) {
                    if (clientData[party].peer == 0) {
                        event.peer->data = (void*)& clientData[party];
                        clientData[party].peer = event.peer;
                        assignedParty = (Party)party;
                        break;
                    }
                }
                if (assignedParty != PN) { // client was accepted as a player
                    enet_peer_send(event.peer, 1,
                        enet_packet_create(&assignedParty, sizeof(Party), ENET_PACKET_FLAG_RELIABLE, PTYPE_PARTY_ASSIGN));
                }
                else {
                    // todo tell client he was not accepted as a player
                }
            } break;
            case ENET_EVENT_TYPE_RECEIVE:
                switch (enet_packet_type(event.packet)) {
                case PTYPE_TIME_SYNC:
                {
                    double time[2] = { *(double*)enet_packet_data(event.packet), glfwGetTime() };
                    printf("timepacket received %.1f %.1f \n", time[0], time[1]);
                    ENetPacket* packet = enet_packet_create(&time, sizeof(double) * 2, 0, PTYPE_TIME_SYNC); // ENET_PACKET_FLAG_RELIABLE
                    // send packet to peer over channel 0
                    enet_peer_send(event.peer, 0, packet);
                    enet_host_flush(host);
                } break;
                case PTYPE_COMPLETE: // send requestet game-sync
                {
                    size_t size;
                    void* d = game.packData(size, glfwGetTime());
                    ENetPacket* packet = enet_packet_create(d, size, ENET_PACKET_FLAG_RELIABLE, PTYPE_COMPLETE);
                    enet_peer_send(event.peer, 1, packet);
                    free(d);
                } break;
                case PTYPE_GAME_CONFIG:
                { // send game config to client
                    Game::GameConfig config = game.getConfig();
                    ENetPacket* packet = enet_packet_create(&config, sizeof(Game::GameConfig), ENET_PACKET_FLAG_RELIABLE, PTYPE_GAME_CONFIG);
                    enet_peer_send(event.peer, 1, packet);
                } break;
                case PTYPE_SHIPS_MOVE:
                {
                    printf("received shipmove event\n");
                    Party commandingParty = ((ClientData*)event.peer->data)->party;
                    game.sendShips(commandingParty, enet_packet_data(event.packet));
                } break;
                case PTYPE_PLANET_ACTION:
                {

                } break;
                case PTYPE_TEXT:
                {

                } break;
                case PTYPE_READY:
                {
                    bool formerReadyState = allClientsReady(clientData, PN);
                    // client tells: he is (not) ready
                    ((ClientData*)event.peer->data)->isReady = *(bool*)enet_packet_data(event.packet);
                    paused = !allClientsReady(clientData, PN);
                    broadcastIfReadystateChanged(host, formerReadyState, !paused);
                } break;
                default:;
                }

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                printf("client party=%zd disconnected.\n", (size_t)((ClientData*)event.peer->data)->party);

                bool formerReadyState = allClientsReady(clientData, PN);
                /* Reset the peer's client information. */
                if (event.peer->data != nullptr) {
                    ((ClientData*)event.peer->data)->peer = nullptr;
                    ((ClientData*)event.peer->data)->isReady = false;
                    event.peer->data = nullptr;
                }
                // broadcast changes if necessary
                paused = !allClientsReady(clientData, PN);
                broadcastIfReadystateChanged(host, formerReadyState, !paused);
            } break;
            case ENET_EVENT_TYPE_NONE:
                break;
            default:;
            }
        }
    }

    glfwDestroyWindow(info.window);


    enet_host_destroy(host);
    return 0;
}

static void cursor_pos_callback(GLFWwindow * window, double xpos, double ypos) {
    mouseR.x = (float)xpos;
    mouseR.y = (float)ypos;
    mouseV.x = mouseR.x + view.x;
    mouseV.y = mouseR.y + view.y;
}

/**
checks if all clients are read (ClientData.isReady==true)
*/
bool allClientsReady(ClientData * clientData, const size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (!clientData[i].isReady)
            return false;
    }
    return true;
}
/**
returns the new paused-state
*/
void broadcastIfReadystateChanged(ENetHost * host, const bool formerReadyState, const bool newReadyState) {
    if (formerReadyState != newReadyState) { // sth changed
        PacketType command;
        if (newReadyState) { // all ready
            command = PTYPE_START;
        }
        else { // at least one is not ready
            command = PTYPE_PAUSE;
        }
        // tell if game continues/pauses
        double time = glfwGetTime();
        ENetPacket* packet = enet_packet_create(&time, sizeof(time), ENET_PACKET_FLAG_RELIABLE, command);
        enet_host_broadcast(host, 0, packet);
    }
}
// eof