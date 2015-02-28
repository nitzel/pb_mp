#ifndef __GAME__
#define __GAME__

#include "inc.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdio.h>


class Game {
  public:
    saPlanet mPlanets;
    saShot mShots[2];
    saShip mShips[2];
    // todo list deadShips, newShips, newShots
    
    unsigned int treeW = 0;
    unsigned int treeH = 0;
    sSquare *mTree = nullptr;
  
  public:
    Game(const unsigned int MAX_SHIPS, const unsigned int NUM_PLANETS);
    void update(const double dt);
    void generateTree();
    void letShoot();
    void letCollide();
    
    void * packData(unsigned int & size, double time); // 
    // returns timeDelta
    double unpackData(void * const data, unsigned int & size, const double time); //

    void select(int clickXY);
    void select(int rectangleXY, int rectangleWH);
};

extern float money[]; // money of PA and PB


#endif // __GAME__
