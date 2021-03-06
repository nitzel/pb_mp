#include "draw.hpp"

// Include stb_image for loading images once(!) from a cpp(!) file
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <cstdint>

struct sInfo info;

const char* textureNames[] = { "res/mFont.png","res/planet.png","res/ship.png","res/ship_marker.png" };

void initGlfw(const char* title, const int screenW, const int screenH, bool fullscreen) {
    if (!glfwInit())
        exit("Failed at glfwInit()\n", EXIT_FAILURE);
    atexit(glfwTerminate);
    glfwSetErrorCallback(cb_error);
    //struct sInfo & info = *getInfo(); // todo decide if necessary

    GLFWmonitor *monitorForFullscreen = fullscreen ? glfwGetPrimaryMonitor() : nullptr; // nullptr = windowed

    info.window = glfwCreateWindow(screenW, screenH, title, monitorForFullscreen, nullptr);
    if (!info.window) {
        exit("Failed at glfwCreateWindow()", EXIT_FAILURE);
    }

    glfwMakeContextCurrent(info.window);
    glViewport(0, 0, (GLsizei)screenW, (GLsizei)screenH);
    // change to projection matrix, reset the matrix and set upt orthogonal view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenW, screenH, 0, 0, 1);// parameters: left, right, bottom, top, near, far

    // ---- OpenGL settings
    glfwSwapInterval(1); // lock to vertical sync of monitor (normally 60Hz/)
    glEnable(GL_SMOOTH); // enable (gouraud) shading
    glEnable(GL_LINE_SMOOTH); // enable (gouraud) shading for lines
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // take the best :)
    glDisable(GL_DEPTH_TEST); // disable depth testing
    glEnable(GL_BLEND); // enable blending (used for alpha)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set blending mode
}

void initGfx() {
    // -------- load images to info
    loadTextures();

    // make a cursor
    std::uint32_t pixels[15][15];
    memset(pixels, 0x00, sizeof(pixels));
    for (size_t i = 0; i < 15; i++) {
        pixels[0][i] = 0xffffffff;        // vert1
        pixels[14][i] = 0xffffffff;        // vert2
        pixels[i][0] = 0xffffffff;        // horizontal1
        pixels[i][14] = 0xffffffff;        // horizontal2
        pixels[i][i] = 0xffffffff;        // diag1
        pixels[i][15 - i - 1] = 0xffffffff;   // diag2
    }

    GLFWimage image;
    image.width = 15;
    image.height = 15;
    image.pixels = (unsigned char*)pixels;
    GLFWcursor* cursor = glfwCreateCursor(&image, 8, 8);
    glfwSetCursor(info.window, cursor);
    // cursor done
}

void drawTree(sSquare* tree, const size_t W, const size_t H, const float dX, const float dY) {
    if (tree == nullptr) return;
    // draw lines
    const size_t WH = W * H;

    glBegin(GL_LINES);
    glColor3ub(255, 255, 0);
    for (size_t x = 0; x < W; x++) {
        for (size_t y = 0; y < H; y++) {
            // vertical
            glVertex2i(dX + x * GRID_SIZE, dY + y * GRID_SIZE);
            glVertex2i(dX + x * GRID_SIZE, dY + (y + 1) * GRID_SIZE);
            // horizontal
            glVertex2i(dX + x * GRID_SIZE, dY + y * GRID_SIZE);
            glVertex2i(dX + (x + 1) * GRID_SIZE, dY + y * GRID_SIZE);

        }
    }
    glEnd();
    // data
    int dx = (int)dX;
    int dy = (int)dY;
    for (size_t party = PA; party < PN; party++) {
        for (size_t x = 0; x < W; x++) {
            for (size_t y = 0; y < H; y++) {
                glColor3ub(party * 255, 255 - party * 255, 0);
                drawInt(tree[party * WH + x * H + y].size, dx + x * GRID_SIZE, dy + y * GRID_SIZE + party * 15);
            }
        }
    }

}

void drawPlanets(const saPlanet & sPlanets, const float dX, const float dY) {
    sPlanet* planets = (sPlanet*)(const char*)sPlanets.planets;
    // draw Planet Texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, info.textures[TEX_PLANET].id);
    glBegin(GL_QUADS);
    const int R = PLANET_RADIUS; // radius

    for (size_t i = 0; i < sPlanets.size; i++) {
        if (planets[i].shieldActive) { // draw a shielded planet!
            glColor3ub(planets[i].party * 150, 150 - planets[i].party * 150, 255);
        }
        else {
            glColor3ub(100 + planets[i].party * 155, 255 - planets[i].party * 155, 100);
        }
        glTexCoord2f(0.0, 0.0); glVertex2i(dX + planets[i].x - R, dY + planets[i].y - R);
        glTexCoord2f(1.0, 0.0); glVertex2i(dX + planets[i].x + R, dY + planets[i].y - R);
        glTexCoord2f(1.0, 1.0); glVertex2i(dX + planets[i].x + R, dY + planets[i].y + R);
        glTexCoord2f(0.0, 1.0); glVertex2i(dX + planets[i].x - R, dY + planets[i].y + R);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    // draw line to gathering point for created ships
    glBegin(GL_LINES);
    glColor3ub(100, 100, 100);
    for (size_t i = 0; i < sPlanets.size; i++) {
        glVertex2i(dX + planets[i].x, dY + planets[i].y);
        glVertex2i(dX + planets[i].tx, dY + planets[i].ty);
    }
    glEnd();
    // draw Health and other numbers onto planet
    for (size_t i = 0; i < sPlanets.size; i++) {
        glColor3ub(250, 250, 25);
        drawInt(planets[i].party, dX + planets[i].x, dY + planets[i].y - R, 1);
        drawInt(planets[i].level[0], dX + planets[i].x - R + 20, dY + planets[i].y - R / 2, 1);
        drawInt(planets[i].level[1], dX + planets[i].x - R + 30, dY + planets[i].y - R / 2, 1);
        drawInt(planets[i].level[2], dX + planets[i].x - R + 40, dY + planets[i].y - R / 2, 1);
        glColor3ub(0, 200, 0);
        drawInt(planets[i].health, dX + planets[i].x - R + 10, dY + planets[i].y, 1);
        glColor3ub(0, 0, 200);
        drawInt(planets[i].power, dX + planets[i].x, dY + planets[i].y, 1);
        glColor3ub(100, 0, 200);
        drawString(planets[i].shieldActive ? "(O)" : " O ", 3, dX + planets[i].x, dY + planets[i].y + R / 2, 1);
        glColor3ub(200, 255, 200);
        drawInt(planets[i].shipQueue, dX + planets[i].x - R / 2, dY + planets[i].y + R / 2, 1);
    }
}
void drawShips(const saShip * sShips, const float dX, const float dY) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, info.textures[TEX_SHIP].id);
    glBegin(GL_QUADS);
    for (size_t party = 0; party < PN; party++) {
        sShip* ships = sShips[party].ships;
        for (size_t i = 0; i < sShips[party].size; i++) {
            if (ships[i].health > 0) {
                glColor3ub(party * 255, (1 - party) * 255, (10 - ships[i].health) * 25);
                glTexCoord2f(0.0, 0.0); glVertex2f(dX + ships[i].x - SHIP_RADIUS, dY + ships[i].y - SHIP_RADIUS);
                glTexCoord2f(1.0, 0.0); glVertex2f(dX + ships[i].x + SHIP_RADIUS, dY + ships[i].y - SHIP_RADIUS);
                glTexCoord2f(1.0, 1.0); glVertex2f(dX + ships[i].x + SHIP_RADIUS, dY + ships[i].y + SHIP_RADIUS);
                glTexCoord2f(0.0, 1.0); glVertex2f(dX + ships[i].x - SHIP_RADIUS, dY + ships[i].y + SHIP_RADIUS);
            }
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
void drawShipMarkers(const sShip * const ships, std::vector<enet_uint32> & selectedShips, const float dX, const float dY) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, info.textures[TEX_SHIP_MARKER].id);
    glBegin(GL_QUADS);
    glColor3ub(255, 255, 255); // always white
    //glColor3ub(75, 255, 75);
    for (size_t i : selectedShips) {
        if (ships[i].health > 0) {
            //glColor3ub(party * 255, (1 - party) * 255, (10 - ships[i].health) * 25);
            glTexCoord2f(0.0, 0.0); glVertex2f(dX + ships[i].x - SHIP_RADIUS, dY + ships[i].y - SHIP_RADIUS);
            glTexCoord2f(1.0, 0.0); glVertex2f(dX + ships[i].x + SHIP_RADIUS, dY + ships[i].y - SHIP_RADIUS);
            glTexCoord2f(1.0, 1.0); glVertex2f(dX + ships[i].x + SHIP_RADIUS, dY + ships[i].y + SHIP_RADIUS);
            glTexCoord2f(0.0, 1.0); glVertex2f(dX + ships[i].x - SHIP_RADIUS, dY + ships[i].y + SHIP_RADIUS);
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawShots(const saShot * sShots, const float dX, const float dY) {
    //glDisable(GL_BLEND); // BLEND because of GL_LINE_SMOOTH, otherwise can be turned off
    glBegin(GL_LINES);
    for (size_t party = 0; party < PN; party++) {
        glColor3ub(party * 255, 255 - party * 255, 0);
        sShot* shots = sShots[party].shots;
        for (size_t i = 0; i < sShots[party].size; i++) {
            if (shots[i].timeToLive > 0) {
                glVertex2f(dX + shots[i].x, dY + shots[i].y);
                glVertex2f(dX + shots[i].x + shots[i].dx * SHOT_LENGTH / SHOT_SPEED, dY + shots[i].y + shots[i].dy * SHOT_LENGTH / SHOT_SPEED);
                //drawInt(i, shots[i].x, shots[i].y); // draw ID instead, glEnd needed!
            }
        }
    }
    glEnd();
    //glEnable(GL_BLEND);
}


void drawRectangle(vec2 v1, vec2 v2, const float dX, const float dY) {
    glBegin(GL_LINE_STRIP);
    glColor3ub(255, 255, 255);
    glVertex2i(dX + v1.x, dY + v1.y); // start upper left
    glVertex2i(dX + v2.x, dY + v1.y); // go upper right
    glVertex2i(dX + v2.x, dY + v2.y); // go lower right
    glVertex2i(dX + v1.x, dY + v2.y); // go lower left
    glVertex2i(dX + v1.x, dY + v1.y); // to beginning

    glEnd();
}

void loadTextures() {
    for (int i = 0; i < TEX_AMOUNT; i++)
        info.textures[i] = loadTexture(textureNames[i]);
}
struct sTexture loadTexture(const char* file) {
    struct sTexture tex;
    unsigned char* data = stbi_load(file, &tex.w, &tex.h, &tex.n, 0);
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

struct sInfo* getInfo() {
    return &info;
}
// Exits program with status and printed error message
void exit(const char* msg, int status) {
    printf(msg);
    exit(status);
}
// GLFWs Error-Callback, just printing the error
void cb_error(int error, const char* description) {
    fprintf(stderr, "GLFW-Error(%i):\"%s\"\n", error, description);
}

void drawInt(int i, float strX, float strY, float stretchXY) {
    drawInt(i, strX, strY, stretchXY, stretchXY);
}
void drawInt(int i, float strX, float strY, float stretchX, float stretchY) {
    char s[12];
    snprintf(s, 12, "%i", i);
    drawString(s, strlen(s), strX, strY, stretchX, stretchY);
}
void drawString(const char* str, size_t strlen, float strX, float strY, float stretchXY) {
    drawString(str, strlen, strX, strY, stretchXY, stretchXY);
}
void drawString(const char* str, size_t strlen, float strX, float strY, float stretchX, float stretchY) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, info.textures[TEX_FONT].id);               // Select Our Texture
    glBegin(GL_QUADS);
    for (size_t i = 0; i < strlen; i++) {
        char c = str[i];
        float cx = ((c - ' ') % 16) / 16.0f; // 16 chars per row, relative to width=1.0f
        float cy = ((c - ' ') / 16) / 6.0f; // 16 chars per row, 6 per col, relative to height=1.0f
        float dx = 8 / 128.0f; // width one char:   8px of 128px per row
        float dy = 16 / 102.0f; // height one char: 16px of  102px per col
        float sx = strX + i * 8 * stretchX; //screenX - where to plot
        float sy = strY;     //screenY

        glTexCoord2f(cx, cy);
        glVertex2d(sx, sy); // ul

        glTexCoord2f(cx + dx, cy);
        glVertex2d(sx + 8 * stretchX, sy); // ur

        glTexCoord2f(cx + dx, cy + dy);
        glVertex2d(sx + 8 * stretchX, sy + 16 * stretchY); // or

        glTexCoord2f(cx, cy + dy);
        glVertex2d(sx, sy + 16 * stretchY); // ol
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

