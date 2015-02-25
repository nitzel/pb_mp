#ifndef __INC__
#define __INC__

#include <forward_list>

#define UPGRADE_MAX_LVL       10    // max level for upgrades
#define UPGRADE_COSTS         50    // cost per upgrade lvl
#define POWER_MAX            100    // maximum power storable
#define POWER_REGEN            1.f  // power regeneration if shield is inactive
#define POWER_DRAIN           10.f  // power drain if shield is active
#define HEALTH_MAX           100    // health maximum for planets
#define HEALTH_REGEN           1.f  // health regeneration
#define MONEY_GEN              1.0f // money per planet per level per second
#define SHIP_PROD_TIME         .50f // time to produce a ship
#define PLANET_RADIUS         64    // planet's radius
#define SEND_SHIP_RAND_RADIUS PLANET_RADIUS    // new ships go in this radius around planet.txy 
#define SHIP_COSTS            10    // credits to pay per ship


#define SHIP_HEALTH_MAX       10    // Maximum health of a ship
#define SHIP_SHOOT_DELAY       1.0f // time between shots/reload

#define SHIP_SPEED           160    // ship-speed, pixel per second
#define SHIP_AIM_RANGE       500    // aiming range of ships. within they can target other ships
#define SHIP_AIM_RANGE_SQ SHIP_AIM_RANGE*SHIP_AIM_RANGE    // aiming range squared
#define SHIP_RADIUS            8

#define SHOT_SPEED           640    // shot-speed pixel per second
#define SHOT_LIFETIME          1.    // timeToLive of a shot, in sec
#define SHOT_LENGTH           16

#define GRID_SIZE            100    //

enum Party{PA=0,PB,PN};
enum Upgrade{ECONOMY=0,DEFENSE,PRODUCTION};

struct vec2 { // for rectangles and coordinates etc
  union {float x, w;};
  union {float y, h;};
};
extern struct vec2 map, screen, view, mouseR, mouseV;

struct sShip {
  double timeToShoot;      // when we can fire the next
  float x,y;   // position
  float dx, dy;  // deltaXY to get to target
  float tx, ty;// target coordinate
  signed char health; // health, zero is dead, below zero is marked for deletion
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
  signed char level[3]; // level Economy, Resistance, 
  unsigned char party;    // PlayerA/B/Neutral
  unsigned short shipQueue; // number of ships in queue
  float health;
  float power;
  unsigned char shieldActive;
};

struct saShip {
  unsigned int freePush;// where to save index in free-array
  unsigned int freePop; // where to take index in free-array
  unsigned int size;
  sShip * ships;
  // how it works
  // In the ships array all ships are stored. To know where to
  // insert a new ship, we store the free indices in the free array.
  // Taking an indice from the free array at freePop gives you an 
  // unused place in ships, while you can insert indices of unused
  // ships at freePush.
  unsigned int * free; // free positions in the ships array
};
struct saPlanet {
  unsigned int size;
  sPlanet * planets;
};
struct saShot {
  unsigned int insertPos;
  unsigned int size;
  sShot * shots;
};
// to partition the map
struct sSquare {
  unsigned int size;
  std::forward_list<sShip*> shiplist; 
};

/// random numbers
float randf();
int rand(int min, int max);
unsigned int rand(unsigned int max);
unsigned long xorshf96(void);


#endif // __INC__
