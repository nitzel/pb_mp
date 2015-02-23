#ifndef __GAME__
#define __GAME__

#include "inc.hpp"

#include <cmath>
#include <cstring>
#include <stdio.h>


extern float money[]; // money of PA and PB

/// init list of game objects
void initGame(saPlanet & planets, saShip * ships, saShot * shots, const unsigned int MAX_SHIPS);
void initPlanets(saPlanet & planets, unsigned int size);
void initShips(saShip & ships, unsigned int size);
void initShots(saShot & shots, unsigned int size);
/// process list of game objects
void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt);
void processShips(saShip * sShips, double dt);
void processShots(saShot * sShots, double dt);

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
