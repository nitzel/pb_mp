#ifndef __GAME__
#define __GAME__

#include "inc.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdio.h>


class Game {
  public:
    vec2 map;
    saPlanet mPlanets;
    saShot mShots[2];
    saShip mShips[2];
    // todo list deadShips, newShips, newShots
    
    unsigned int treeW = map.w/GRID_SIZE;
    unsigned int treeH = map.h/GRID_SIZE;
    sSquare *mTree;
  
  public:
    Game(const unsigned int MAX_SHIPS, const unsigned int NUM_PLANETS);
    void update(const double dt);
    void generateTree();
    void letShoot();
    void letCollide();
    
    void * packData(unsigned int & size); // 
    void unpackData(unsigned int & size); //

    void select(int clickXY);
    void select(int rectangleXY, int rectangleWH);
};

extern float money[]; // money of PA and PB

/// init list of game objects
void initGame(saPlanet & planets, saShip * ships, saShot * shots, const unsigned int MAX_SHIPS);
void initPlanets(saPlanet & planets, const unsigned int size);
void initShips(saShip & ships, const unsigned int size);
void initShots(saShot & shots, const unsigned int size);
/// process list of game objects
void updatePlanets(saPlanet & sPlanets, saShip * sShips, const double dt);
void updateShips(saShip * sShips, const double dt);
void updateShots(saShot * sShots, const double dt);

/**
ship/planet - ship/planet to check for closest enemy and shoot
sPlanets - list of planets
shots - list of shots from the same party as ships
rivalTree - spacepartioning tree with the rival-partys ships
W,H - Size of rivalTree
*/
// todo split into shoot and collision detection, also separate the drawTree call from here 
bool shoot(sShip & ship, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H, const unsigned int PARTY);
bool shoot(sPlanet & planet, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H);
void shoot(saShip * sShips, saPlanet & sPlanets, saShot * sShots,double dt);


/// helping functions
inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy); /// dx=tx-x
inline void normalize(float & x, float & y, const float LEN);  /// normalized vector * LEN
inline unsigned int distanceSQ(const float x, const float y, const float x2, const float y2); /// distance^2
/// command ship/shot to go somewhere
void flyToTarget(sShot & shot, const float tx, const float ty);
void flyToTarget(sShip & ship, const float tx, const float ty);
/// insert or delete ships or shots
bool addShot(saShot & shots, const float x, const float y, const float tx, const float ty);
bool addShip(saShip & ships, const float x, const float y, const float tx, const float  ty);
void deleteShip(saShip & ships, const unsigned int id);
/// damage ships or planets
void takeDamage(sShip & ship);
void takeDamage(sPlanet & planet, const unsigned int party);
void capturePlanet(sPlanet & planet, const unsigned int newParty);
/// upgrade planets
bool upgradePlanet(sPlanet & planet, Upgrade upgrade);




#endif // __GAME__
