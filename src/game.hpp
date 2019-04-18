#ifndef __GAME__
#define __GAME__

#include "inc.hpp"

#include <enet/types.h>

class Game {
public:
    struct GameConfig {
        enet_uint32 numPlanets;
        enet_uint32 numShips;
        enet_uint32 numShots;
        vec2 map;
        float money[2];
    };
    static GameConfig createConfig(size_t const NUM_PLANETS, size_t const NUM_SHIPS, vec2 const map, float moneyA = 0, float moneyB = 0);
public:
    float mMoney[PN]; // money of PA and PB
    vec2 mMap;

    saPlanet mPlanets;
    saShot mShots[2];
    saShip mShips[2];

    size_t treeW = 0;
    size_t treeH = 0;
    sSquare* mTree = nullptr;

    std::vector<enet_uint32> selectedShips;
private:
    GameConfig config;
public:
    Game(vec2 map, const size_t MAX_SHIPS, const size_t NUM_PLANETS);
    Game(GameConfig cfg);
    ~Game();

    GameConfig getConfig();

    void update(const double dt, const bool bUpdatePlanets = true);
    void shootAndCollide();

    void* packData(size_t& size, double time); // 
    double unpackData(void* const data, size_t size, const double time); // returns timeDelta

    void* packUpdateData(size_t& size, double time); // 
    double unpackUpdateData(void* const data, size_t size, const double time); // returns timeDelta

    void select(Party party, vec2 v);
    void select(Party party, vec2 v1, vec2 v2);
    /// command all selected ships to go somewhere
    void* sendSelectedGetData(Party party, vec2 v1, vec2 v2, size_t formation, size_t& size);
    void sendShips(Party party, void* const data);

    void clearChanged();

    void generateTree();
    void letShoot();
    /* dealDamage=false: Shots don't deal any damage but vanish (use on client) */
    void letCollide(bool dealDamage);

private:
    /// init list of game objects
    void initPlanets(saPlanet& planets, const size_t size);
    void initShips(saShip& ships, const size_t size);
    void initShots(saShot& shots, const size_t size);
    /// process list of game objects
    void updatePlanets(const double dt);
    void updateShips(const double dt);
    void updateShots(const double dt);
    void updateShotsIntervall(saShot& sShots, const size_t pStart, const size_t num, const double dt);
    void updateShip(sShip& ship, const double dt); // single ship update
    /**
    ship/planet - ship/planet to check for closest enemy and shoot
    sPlanets - list of planets
    shots - list of shots from the same party as ships
    rivalTree - spacepartioning tree with the rival-partys ships
    W,H - Size of rivalTree
    */
    // todo split into shoot and collision detection, also separate the drawTree call from here 
    bool shoot(sShip& ship, saPlanet& sPlanets, saShot& shots, sSquare* rivalTree, const size_t W, const size_t H, Party party);
    bool shoot(sPlanet& planet, saPlanet& sPlanets, saShot& shots, sSquare* rivalTree, const size_t W, const size_t H);
    /// helping functions
    inline double vecLen(const vec2 v);
    inline void delta(const float x, const float y, const float tx, const float ty, float& dx, float& dy); /// dx=tx-x
    inline void normalize(float& x, float& y, const float LEN);  /// normalized vector * LEN
    inline size_t distanceSQ(const float x, const float y, const float x2, const float y2); /// distance^2
    /// command ship/shot to go somewhere
    void flyToTarget(sShot& shot, const float tx, const float ty);
    void flyToTarget(Party party, const size_t id, const float tx, const float ty);
    /// insert or delete ships or shots
    bool addShot(Party party, const float x, const float y, const float tx, const float ty);
    bool addShip(Party party, const float x, const float y, const float tx, const float  ty);
    void deleteShip(Party party, const size_t id);
    /// damage ships or planets
    void takeDamage(sShip& ship);
    void takeDamage(sPlanet& planet, const size_t party);
    void capturePlanet(sPlanet& planet, const size_t newParty);
    /// upgrade planets
    bool upgradePlanet(sPlanet& planet, Upgrade upgrade);



    /// un/packing gameData
    void* packChangedShips(Party party, enet_uint32& size);
    void unpackChangedShips(Party party, void* const data, const double dt);
    void* packChangedShots(Party party, enet_uint32& size);
    void unpackChangedShots(Party party, void* const data, const double dt);
};


#endif // __GAME__
