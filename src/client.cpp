#include "client.hpp"

#include "game.hpp"
#include "draw.hpp"
#include "net.hpp"
#include "configuration.hpp"

#include <iostream>

// just about mouseclicks/releases!
#include <string>
int mouseChanged[8]{ -1,-1,-1,-1,-1,-1,-1,-1 };  // -1 nothing, GLFW_PRESS or GLFW_RELEASE
vec2 mouseStates[8][4]; // mousebutton 0-7, posDown(R), posUp(R), posDown(Virtual), posUp

static void callback_mouseMove(GLFWwindow* window, double xpos, double ypos);
static void updateMouseOnMapPosition(const vec2 mouseRelativeToWindow, const vec2 viewPosition);
static void callback_mouseButton(GLFWwindow* window, int button, int action, int mods);
static void callback_mouseScroll(GLFWwindow* window, double dx, double dy);


#define TIME_TO_SYNC_TIME 0.5f
#define GAMESTATE_OLD 100.f // dont care about age! 0.5f // gamestates older than this will be discarded
int client(const CConfiguration& config) {
    std::cout << "Client" << ", server at " << config.client.host << ":" << config.client.port << std::endl;

    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetHost* host;
    ENetPeer* peer;
    ENetAddress address;
    ENetEvent event;

    // client
    host = enet_host_create(NULL, 1, 3, 0, 0); // 3 channels
    if (host == NULL) {
        printf("an error occured while trying to create an enet client.\n");
        exit(EXIT_FAILURE);
    }
    enet_host_bandwidth_throttle(host);
    enet_address_set_host(&address, config.client.host.c_str());
    address.port = config.client.port;

    peer = enet_host_connect(host, &address, 3, 0); // 3 channels

    if (peer == nullptr) {
        printf("no available peers for initiating an enet connection\n");
        exit(EXIT_FAILURE);
    }

    if (enet_host_service(host, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        printf("Connection to host succeeded\n");
    else {
        enet_peer_reset(peer);
        printf("Connection to host failed\n");
        exit(EXIT_FAILURE); // todo try reconnect
    }
    enet_host_flush(host);

    mouseR = { 0,0 };
    mouseV = { 0,0 };
    screen = { (float)config.graphics.width, (float)config.graphics.height };
    view = { 0,0 };
    Game* game = nullptr;
    ///////////////////////////////
    // init GLFW
    /////////////////////////////////s
    initGlfw("PB-MP-Client", (int)screen.x, (int)screen.y, config.graphics.fullscreen);
    initGfx();
    // add input listeners
    glfwSetCursorPosCallback(info.window, callback_mouseMove);
    glfwSetMouseButtonCallback(info.window, callback_mouseButton);
    glfwSetScrollCallback(info.window, callback_mouseScroll);
    ////////////////
    // VARS
    ////////////////
    Party party = PN; // shall be overwritten later!
    double time = glfwGetTime();
    double dt = 0, vdt = 0; // virtuel dt, added to dt on data-arrival
    double fps = 0;
    bool paused = true;
    double timeToSyncT = 0;

    bool timeSynced = false;
    while (!glfwWindowShouldClose(info.window)) {
        // update timer
        dt = glfwGetTime() - time;
        time = glfwGetTime();
        fps = (fps * 500 + 10 / dt) / 510;


        // sync time
        timeToSyncT -= dt;
        if (timeToSyncT < 0)
        {
            timeToSyncT = TIME_TO_SYNC_TIME; // new timer for sync
            double pTime[2] = { glfwGetTime(),0 };
            ENetPacket* packet = enet_packet_create((void*)pTime, 2 * sizeof(double), 0, PTYPE_TIME_SYNC); // ENET_PACKET_FLAG_RELIABLE
            enet_peer_send(peer, 0, packet);

            if (!game && timeSynced) { // if synced, ask for gameconfig
                ENetPacket* packetGR = enet_packet_create(nullptr, 0, ENET_PACKET_FLAG_RELIABLE, PTYPE_GAME_CONFIG);
                enet_peer_send(peer, 1, packetGR);
            }
            enet_host_flush(host);
        }

        //////////////////
        // Handle input //
        //////////////////
        if (game) {
            // evaluate mouseactions
            if (mouseChanged[GLFW_MOUSE_BUTTON_1] == GLFW_RELEASE) { // left up
                vec2 vp = mouseStates[GLFW_MOUSE_BUTTON_1][GLFW_PRESS + 2]; // press coords
                vec2 vr = mouseStates[GLFW_MOUSE_BUTTON_1][GLFW_RELEASE + 2]; // release coords
                if (vp.x != vr.x || vp.y != vr.y) { // range select
                    game->select(party, vp, vr);
                }
                else { // just a click
                    game->select(party, vp);
                }
                mouseChanged[GLFW_MOUSE_BUTTON_1] = -1; // mark as read
            }
            if (mouseChanged[GLFW_MOUSE_BUTTON_2] == GLFW_RELEASE) { // right up
                vec2 vp = mouseStates[GLFW_MOUSE_BUTTON_2][GLFW_PRESS + 2]; // press coords
                vec2 vr = mouseStates[GLFW_MOUSE_BUTTON_2][GLFW_RELEASE + 2]; // release coords
                //if(vp.x != vr.x || vp.y != vr.y){ // todo: formations
                  //game->select(vp, vr);
                //} else { // just a click, send to target
                size_t formation = 0; // todo circle, square, rect etc...
                size_t size = 0;
                void* data = game->sendSelectedGetData(party, vp, vr, formation, size);
                if (data != nullptr) {
                    // game->sendShips(party, data); // todo see Issue#16 improve/leave/remove
                    ENetPacket* packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE, PTYPE_SHIPS_MOVE);
                    enet_peer_send(peer, 1, packet);
                    free(data);
                }
                //}
                mouseChanged[GLFW_MOUSE_BUTTON_2] = -1; // mark as read
            }

            // move view
            float dx = 0, dy = 0;
            if (mouseR.x < 80) dx = mouseR.x - 80;
            if (mouseR.y < 80) dy = mouseR.y - 80;
            if (mouseR.x > screen.w - 80) dx = mouseR.x - (screen.w - 80);
            if (mouseR.y > screen.h - 80) dy = mouseR.y - (screen.h - 80);
            view.x += dx / 80.f * 20;
            view.y += dy / 80.f * 20;
            if (dx != 0 || dy != 0) {
                if (view.x < 0) view.x = 0;
                if (view.y < 0) view.y = 0;
                if (view.x > game->mMap.w - screen.w) view.x = game->mMap.w - screen.w;
                if (view.y > game->mMap.h - screen.h) view.y = game->mMap.h - screen.h;
                updateMouseOnMapPosition(mouseR, view);
            }
        }
        // process game content
        if (!paused && game) {
            game->clearChanged();
            game->update(dt + vdt, false); // dont update planets as client
            game->generateTree();
            game->letCollide(false); // todo activate later :) shots shouldn't make damage but vanish when hitting sth
            vdt = 0;
        }

        ///////////////////
        // Graphic STUFF //
        ///////////////////
        // clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW); // reset the matrix
        glLoadIdentity();
        // draw gamecontent
        if (game) {
            drawPlanets(game->mPlanets, -view.x, -view.y);
            drawShipMarkers(game->mShips[party].ships, game->selectedShips, -view.x, -view.y);
            drawShips(game->mShips, -view.x, -view.y);
            drawShots(game->mShots, -view.x, -view.y);
            drawTree(game->mTree, game->treeW, game->treeH, -view.x, -view.y);
            if (mouseChanged[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS) { // if dragging, draw rectangle
                drawRectangle(mouseStates[GLFW_MOUSE_BUTTON_1][2 + GLFW_PRESS], mouseV, -view.x, -view.y); // draw rectangle, virtual one (2+0/1)
            }
            char s[100];
            snprintf(s, 100, "FPS=%4.1f=%2.1f t=%.1fs Money A=%5i B=%5i MR%i/%i MV%i/%i View%i/%i", fps, 1 / dt, time, (int)game->mMoney[PA], (int)game->mMoney[PB], (int)mouseR.x, (int)mouseR.y, (int)mouseV.x, (int)mouseV.y, (int)view.x, (int)view.y);
            glColor3ub(255, 255, 255);
            drawString(s, strlen(s), 10, 10);
        }
        glfwSwapBuffers(info.window);
        glfwPollEvents();

        ///////////////////
        // Network STUFF //
        ///////////////////
        while (enet_host_service(host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                //printf("packet received type=%d\n",enet_packet_type(event.packet));
                switch (enet_packet_type(event.packet)) {
                case PTYPE_TIME_SYNC:
                {
                    double* t = (double*)enet_packet_data(event.packet); // time
                    //printf("timepacket received %.1f %.1f \n",t[0],t[1]);
                    glfwSetTime(t[1] + (glfwGetTime() - t[0]) / 2);
                    timeSynced = true;
                } break;
                case PTYPE_PARTY_ASSIGN: // got a party assigned
                {
                    Party newParty = *(Party*)enet_packet_data(event.packet);
                    if (party != PN) { fprintf(stderr, "ERR: MultipePartyAssign: %i assigned, was %i before", newParty, party); } // reporting error
                    party = newParty;
                    printf("new party: %i\n", party);
                } break;
                case PTYPE_COMPLETE: // complete gamestate
                {
                    if (game) {
                        const double t = *(double*)enet_packet_data(event.packet);
                        const double pDt = glfwGetTime() - t; // packet delta time (packet age)
                        printf("bc size=%d servertime=%.2f dt=%.2f\n", (int)enet_packet_size(event.packet), t, pDt);
                        if (pDt < 0) { // packet from "future"
                            fprintf(stderr, "future packet received from t=%.2f at %.2f\n", t, glfwGetTime());
                        }
                        else if (pDt < GAMESTATE_OLD) {  // todo better algorithm than just age! we discard too old packets
                            vdt = game->unpackData(enet_packet_data(event.packet), enet_packet_size(event.packet), glfwGetTime());
                        }
                        // tell server this client is ready
                        bool ready = true;
                        ENetPacket* packet = enet_packet_create(&ready, sizeof(ready), ENET_PACKET_FLAG_RELIABLE, PTYPE_READY);
                        enet_peer_send(peer, 1, packet);
                    }
                } break;
                case PTYPE_UPDATE:
                    if (game) {
                        const double t = *(double*)enet_packet_data(event.packet);
                        const double pDt = glfwGetTime() - t; // packet delta time (packet age)
                        //printf("bc size=%d servertime=%.2f dt=%.2f\n", enet_packet_size(event.packet), t, pDt);
                        if (pDt < 0) { // packet from "future"
                            fprintf(stderr, "future updatepacket received from t=%.2f at %.2f\n", t, glfwGetTime());
                        }
                        // todo: discard old updates or not? they are still important...
                        game->unpackUpdateData(enet_packet_data(event.packet), enet_packet_size(event.packet), glfwGetTime(), party);

                    } break;
                case PTYPE_GAME_CONFIG:
                    if (timeSynced) { // receiving game config
                        printf("Received Game Config\n");
                        // create Game instance
                        game = new Game(*(Game::GameConfig*)enet_packet_data(event.packet));
                        //ask for complete gamesync
                        ENetPacket* packet = enet_packet_create(nullptr, 0, ENET_PACKET_FLAG_RELIABLE, PTYPE_COMPLETE);
                        enet_peer_send(peer, 1, packet);
                    } break;
                case PTYPE_TEXT:
                {

                } break;
                case PTYPE_START:
                {
                    printf("start!\n");
                    paused = false;
                    // time since start...
                    vdt = glfwGetTime() - *(double*)enet_packet_data(event.packet);
                } break;
                case PTYPE_PAUSE:
                {
                    printf("pause!\n");
                    paused = true;
                    // todo time since stop... should be gone backwards
                    //vdt = -(glfwGetTime()-*(double*)enet_packet_data(event.packet));
                } break;
                default:;
                }
                // Clean up the packet now that we're done using it.
                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                // todo what can we do here?
                printf("%s disconnected.\n", (char*)event.peer->data);
                // Reset the peer's client information. 
                event.peer->data = NULL;
                paused = true; // todo remove
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            default:;
            }
        }
    }

    delete game;




    enet_host_flush(host);
    enet_peer_disconnect(peer, 0);
    bool disconnected = false;
    while (enet_host_service(host, &event, 1000) > 0 && !disconnected) {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            puts("Disconnection succeeded.\n");
            disconnected = true;
        default:
            break;
        }
    }
    if (!disconnected) {
        puts("Disconnection failed.\n");
        enet_peer_reset(peer);
    }

    glfwDestroyWindow(info.window);

    glfwTerminate();

    enet_deinitialize();

    return 0;
}

static void updateMouseOnMapPosition(const vec2 mouseRelativeToWindow, const vec2 viewPosition) {
    mouseV = mouseRelativeToWindow + viewPosition;
}

static void callback_mouseMove(GLFWwindow * window, double xpos, double ypos) {
    mouseR.x = (float)xpos;
    mouseR.y = (float)ypos;
    updateMouseOnMapPosition(mouseR, view);
}

static void callback_mouseButton(GLFWwindow * window, int button, int action, int mods) {
    printf("mouseclick btn=%d action=%d mods=%d\n", button, action, mods);
    mouseChanged[button] = action;
    mouseStates[button][action] = mouseR; // save real mouse pos
    mouseStates[button][action + 2] = mouseV; // save virtual mouse pos
}
static void callback_mouseScroll(GLFWwindow * window, double dx, double dy) {
    printf("mousescroll %f   %f\n", dx, dy);
}
