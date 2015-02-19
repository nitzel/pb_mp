#ifndef __INC__
#define __INC__

#include <stdio.h>
#include <string>
#include <math.h>
#include <forward_list>
#include <GLFW/glfw3.h>
#include <enet/enet.h>

#define STBI_NO_HDR
#include "include/stb_image.h"


#define UPGRADE_MAX_LVL  10     // max level for upgrades
#define POWER_MAX       100     // maximum power storable
#define POWER_REGEN       1.f   // power regeneration if shield is inactive
#define POWER_DRAIN      10.f   // power drain if shield is active
#define HEALTH_MAX      100     // health maximum for planets
#define HEALTH_REGEN      1.f   // health regeneration
#define MONEY_GEN         1.0f  // money per planet per level per second
#define SHIP_PROD_TIME   4.0f  // after this time a ship is produced

#define SHIP_HEALTH_MAX  10;
#define SHIP_SHOOT_DELAY  4.0f;


enum Party{PA,PB,PN};
enum Upgrades{ECONOMY,DEFENSE,PRODUCTION};
enum TextureID{TEX_FONT=0,TEX_PLANET,TEX_SHIP,TEX_AMOUNT};
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

struct sShip {
  double timeToShoot;      // when we can fire the next
  float x,y;   // position
  float dx, dy;  // deltaXY to get to target
  float tx, ty;// target coordinate
  unsigned char health; // health, zero is dead
};

struct sShot {              
  double timeToLive;        // dead after max-age
  float x,y;       // position
  float dx,dy;// deltaXY, moving direction
};

struct sPlanet {
  double timeToBuild;
  double timeToShoot;
  float x,y;
  float tx,ty;   // pos to send new shipsProduction
  unsigned char level[3]; // level Economy, Resistance, 
  unsigned char party;    // PlayerA/B/Neutral
  unsigned short shipQueue; // number of ships in queue
  float health;
  float power;
  unsigned char shieldActive;
};

struct saShip {
  sShip * ships;
  // how it works
  // In the ships array all ships are stored. To know where to
  // insert a new ship, we store the free indices in the free array.
  // Taking an indice from the free array at freePop gives you an 
  // unused place in ships, while you can insert indices of unused
  // ships at freePush.
  int * free; // free positions in the ships array
  unsigned int freePush;// where to save index in free-array
  unsigned int freePop; // where to take index in free-array
  unsigned int size;
};
struct saPlanet {
  sPlanet * planets;
  unsigned int size;
};
struct saShot {
  sShot * shots;
  unsigned int insertPos;
  unsigned int size;
};

void drawPlanets(const saPlanet & sPlanets, const float dX, const float dY);
void drawShips(const saShip * ships, const float dX, const float dY);
void drawShots(const saShot * shots, const float dX, const float dY);

void drawInt(int i, float strX, float strY, float stretchX, float stretchY);
void drawInt(int i, float strX, float strY, float stretchXY=1);
void drawString(const char* str, unsigned int strlen, float strX, float strY, float stretchX, float stretchY);
void drawString(const char* str, unsigned int strlen, float strX, float strY, float stretchXY=1);
void loadTextures();
struct sTexture loadTexture(const char * file);
struct sInfo * getInfo();
void exit(const char * msg, int status);
void cb_error(int error, const char * description);

#endif // __INC__