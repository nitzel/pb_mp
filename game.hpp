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
    
    size_t treeW = 0;
    size_t treeH = 0;
    sSquare *mTree = nullptr;
  
  public:
    Game(const size_t MAX_SHIPS, const size_t NUM_PLANETS);
    void update(const double dt, const bool bUpdatePlanets = true);
    void shootAndCollide();
    
    void * packData(size_t & size, double time); // 
    double unpackData(void * const data, size_t size, const double time); // returns timeDelta

    void * packUpdateData(size_t & size, double time); // 
    double unpackUpdateData(void * const data, size_t size, const double time); // returns timeDelta
    
    void select(int clickXY);     // todo!
    void select(int rectangleXY, int rectangleWH);
    
    void clearChanged();
  public:
    void generateTree();
    void letShoot();
    void letCollide();
};

extern float money[]; // money of PA and PB


#endif // __GAME__
