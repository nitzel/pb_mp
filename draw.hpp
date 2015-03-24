#ifndef __DRAW__
#define __DRAW__

#include "inc.hpp"

#include <cstring>
#include <cstdlib>
#include <GLFW/glfw3.h>

#define STBI_NO_HDR
#include "include/stb_image.h"  // for loading images

enum TextureID{TEX_FONT=0,TEX_PLANET,TEX_SHIP,TEX_SHIP_MARKER,TEX_AMOUNT};
extern const char * textureNames[];

struct sTexture{
  GLuint id;
  int w,h,n;
};
struct sInfo {
  GLFWwindow * window;
  sTexture textures[TEX_AMOUNT];
};
extern struct sInfo info;

//////////
// init
void initGlfw(const char * title, const int screenW, const int screenH);
void initGfx();


void drawTree(sSquare* tree, const uint32_t W, const uint32_t H, const float dX, const float dY);
///////////
// draw game content
void drawPlanets(const saPlanet & sPlanets, const float dX, const float dY);
void drawShips(const saShip * ships, const float dX, const float dY);
void drawShipMarkers(const sShip * const ships, std::vector<uint32_t> & selectedShips, const float dX, const float dY);
void drawShots(const saShot * shots, const float dX, const float dY);

void drawRectangle(vec2 v1, vec2 v2, const float dX, const float dY);
/////////
// draw text/numbers/strings
void drawInt(int i, float strX, float strY, float stretchX, float stretchY);
void drawInt(int i, float strX, float strY, float stretchXY=1);
void drawString(const char* str, uint32_t strlen, float strX, float strY, float stretchX, float stretchY);
void drawString(const char* str, uint32_t strlen, float strX, float strY, float stretchXY=1);

////////////////////
//  other stuff
void loadTextures();
struct sTexture loadTexture(const char * file);
struct sInfo * getInfo();
void exit(const char * msg, int status);
void cb_error(int error, const char * description);

#endif // __DRAW__
