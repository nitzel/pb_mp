#ifndef __GAME__
#define __GAME__

#include "inc.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>


class Game {  
  public:
    struct GameConfig {
      /// networked, use constsize types only
      uint32_t numPlanets;
      uint32_t numShips;
      uint32_t numShots;
      vec2 map;
      float money[2];
    };
    static GameConfig createConfig(uint32_t const NUM_PLANETS, uint32_t const NUM_SHIPS, vec2 const map, float moneyA=0, float moneyB=0);
  public:
    float mMoney[PN]; // money of PA and PB
    vec2 mMap;
    
    saPlanet mPlanets;
    saShot mShots[2];
    saShip mShips[2];
    
    uint32_t treeW = 0;
    uint32_t treeH = 0;
    sSquare *mTree = nullptr;
  
    std::vector<uint32_t> selectedShips;
  private:
    void GameCtor(GameConfig cfg);
    GameConfig config;
  public:
    Game(vec2 map, const uint32_t MAX_SHIPS, const uint32_t NUM_PLANETS);
    Game(GameConfig cfg);
    ~Game();
    
    GameConfig getConfig();
    
    void update(const double dt, const bool bUpdatePlanets = true);
    void shootAndCollide();
    
    void * packData(uint32_t & size, double time); // 
    double unpackData(void * const data, uint32_t size, const double time); // returns timeDelta

    void * packUpdateData(uint32_t & size, double time); // 
    double unpackUpdateData(void * const data, uint32_t size, const double time); // returns timeDelta
    
    void select(Party party, vec2 v);
    void select(Party party, vec2 v1, vec2 v2);
    /// command all selected ships to go somewhere
    void * sendSelectedGetData(Party party, vec2 v1, vec2 v2, uint32_t formation, uint32_t & size);
    void sendShips(Party party, void * const data);
    
    void clearChanged();
    
    void generateTree();
    void letShoot();
    void letCollide();
    
  private:
    /// init list of game objects
    void initGame(saPlanet & planets, saShip * ships, saShot * shots, const uint32_t MAX_SHIPS);
    void initPlanets(saPlanet & planets, const uint32_t size);
    void initShips  (saShip   & ships,   const uint32_t size);
    void initShots  (saShot   & shots,   const uint32_t size);
    /// process list of game objects
    void updatePlanets(const double dt);
    void updateShips(const double dt);
    void updateShots(const double dt);
    void updateShotsIntervall(saShot & sShots, const uint32_t pStart, const uint32_t num, const double dt); 
    void updateShip(sShip & ship, const double dt); // single ship update
    /**
    ship/planet - ship/planet to check for closest enemy and shoot
    sPlanets - list of planets
    shots - list of shots from the same party as ships
    rivalTree - spacepartioning tree with the rival-partys ships
    W,H - Size of rivalTree
    */
    // todo split into shoot and collision detection, also separate the drawTree call from here 
    bool shoot(sShip & ship, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const uint32_t W, const uint32_t H, Party party);
    bool shoot(sPlanet & planet, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const uint32_t W, const uint32_t H);
    /// helping functions
    inline double vecLen(const vec2 v);
    inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy); /// dx=tx-x
    inline void normalize(float & x, float & y, const float LEN);  /// normalized vector * LEN
    inline uint32_t distanceSQ(const float x, const float y, const float x2, const float y2); /// distance^2
    /// command ship/shot to go somewhere
    void flyToTarget(sShot & shot, const float tx, const float ty);
    void flyToTarget(Party party, const uint32_t id, const float tx, const float ty);
    /// insert or delete ships or shots
    bool addShot(Party party, const float x, const float y, const float tx, const float ty);
    bool addShip(Party party, const float x, const float y, const float tx, const float  ty);
    void deleteShip(Party party, const uint32_t id);
    /// damage ships or planets
    void takeDamage(sShip & ship);
    void takeDamage(sPlanet & planet, const uint32_t party);
    void capturePlanet(sPlanet & planet, const uint32_t newParty);
    /// upgrade planets
    bool upgradePlanet(sPlanet & planet, Upgrade upgrade);
    
    
    
    /// un/packing gameData
    void * packChangedShips(Party party, uint32_t & size);
    void unpackChangedShips(Party party, void * const data, const double dt);
    void * packChangedShots(Party party, uint32_t & size);
    void unpackChangedShots(Party party, void * const data, const double dt);
};


#endif // __GAME__
