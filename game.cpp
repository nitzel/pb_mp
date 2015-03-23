#include <algorithm>
#include "game.hpp"
#define TREE(X,Y,Z)  mTree[(X*treeW+Y)*treeH+Z] // from XYZ to [x][y][z]
#define TREE1(X)  &mTree[X*treeW*treeH] // from XYZ to [x][y][z]

Game::GameConfig Game::createConfig(size_t const NUM_PLANETS, size_t const NUM_SHIPS, vec2 const map, float moneyA, float moneyB){
  GameConfig cfg;
  cfg.numPlanets = NUM_PLANETS;
  cfg.numShips = NUM_SHIPS;
  cfg.numShots = cfg.numShips*(float)SHOT_LIFETIME/(float)SHIP_SHOOT_DELAY + cfg.numPlanets*50; // + 50/planet just to be sure - that means, a planet can shoot 50 shots within the lifespan of one shot  and we can still store all shots. Also all planets (and they are all captured by one party) fire at the same time and all ships ... will probably not happen, but just to be sure :D
  if(map.x>0 && map.y>0)
    cfg.map = map;
  else  {
    fprintf(stderr, "createcfg: invalid mapsize %f/%f, resized to 1000/1000", map.x, map.y);
    cfg.map = vec2{1000,1000};
  }
  cfg.money[PA] = moneyA;
  cfg.money[PB] = moneyB;
  
  return cfg;
}

void Game::GameCtor(GameConfig cfg){
  config = cfg;
  mMoney[PA] = config.money[PA];
  mMoney[PB] = config.money[PB];
  mMap = config.map;
  
  treeW = mMap.w/GRID_SIZE;
  treeH = mMap.h/GRID_SIZE;
  
  initPlanets(mPlanets,  config.numPlanets);
  initShips(mShips[PA],  config.numShips);
  initShips(mShips[PB],  config.numShips);
  initShots(mShots[PA],  config.numShots);
  initShots(mShots[PB],  config.numShots);  
}
Game::Game(GameConfig cfg){
  GameCtor(cfg);
}
Game::Game(vec2 map, const size_t MAX_SHIPS, const size_t NUM_PLANETS){
  GameCtor(Game::createConfig(NUM_PLANETS, MAX_SHIPS, vec2{2000,2000}));
}

Game::~Game(){
  if(mTree != nullptr) {
    delete[] mTree; 
    mTree = nullptr;
  }
  for(size_t party=0; party<PN; party++){
    delete[] mShots[party].shots;
    delete[] mShips[party].ships;
    delete[] mShips[party].free;
  }
  delete[] mPlanets.planets;
}

void Game::update(const double dt, const bool bUpdatePlanets){
  if(bUpdatePlanets)
    updatePlanets(dt);
  updateShips(dt);
  updateShots(dt);
}

void Game::shootAndCollide() {
  generateTree();
  letShoot();
  letCollide();
}

void Game::generateTree(){
  /// The idea is to split the playground into squares lying next to 
  /// each other, each the size of the aiming-range of the ships.
  /// By coordinates you can directly calculated in which square a ship is
  /// It then has only to check against the 9 squares around.
  ///
  /// If there are :alot: of ships in a square it is further divided 
  /// into smaller ones, until they contain a reasonable amount. 
  /// Only the ones on aim-range will be tested
  
  
  
  // todo improve with several layers, experiment
  // 1st: SHIP_AIM_RANGE^2, containing the amount in the whole Rectangle AND a list of layer2 nodes that are not empty
  // 2nd: Smaller
  // we dont need THE nearest enemy, just a close one!
  
  
  /// Set up Structure
  if(mTree == nullptr)
    mTree = new sSquare[2 * treeW * treeH];
  
  // init size of tree with zero
  for(size_t party=PA; party<PN; party++)
    for(size_t x=0; x<treeW; x++)
      for(size_t y=0; y<treeH; y++){
        TREE(party,x,y).size = 0;
        TREE(party,x,y).shiplist.clear();
      }
  
  /// Fill space partitioning structure with ships
  // todo try to optimize, takes 100fps away
  for(size_t party=PA; party<PN; party++){
    sShip * ships = mShips[party].ships;
    for(size_t i=0; i<mShips[party].size; i++){
      if(ships[i].health>0){
        TREE(party,(size_t)(ships[i].x)/GRID_SIZE,(size_t)(ships[i].y)/GRID_SIZE).size++;
        TREE(party,(size_t)(ships[i].x)/GRID_SIZE,(size_t)(ships[i].y)/GRID_SIZE).shiplist.push_back(i); 
      }
    }
  }
}

void Game::letShoot(){
/////////////////
  // RangeTesting and shooting
  //////////////// 
  // hint: im using a cool algorithm i invented on my own xD
  // commenting might be a bit odd, since it was to let the ships
  // go in a squared without counting how many ships there are
  // therefore we start in the center of the square and going outwards
  // in a spiral way
  // Here it is used for going through the grid, using closer grid
  // parts first to find a good-enough shootable ship (optimal would take too much time, we take the first we can find in reach, which is roughly the closest. the really closest may be about sqrt(GRID_size) closer, which is acceptable)
  for(size_t party=PA; party<PN; party++){
    //sShip * lastTarget = nullptr;
    //float lastTargetDistanceSQ = 0;
    size_t rival = !party; // opponents party ID :)
    // let ships shoot
    for(size_t i=0; i<mShips[party].size; i++){
      sShip & ship = mShips[party].ships[i];
      if(!ship.health || ship.timeToShoot>0) {// dead or weapon not ready
        continue;
      }
      shoot(ship, mPlanets, mShots[party],TREE1(rival),  treeW, treeH, (Party)party);
    }
  }
  // let planets shoot
  for(size_t i=0; i<mPlanets.size; i++){
    sPlanet & planet = mPlanets.planets[i];      
    if(planet.party == PN || planet.timeToShoot>0) {// dead or weapon not ready
      continue;
    }
    shoot(planet, mPlanets, mShots[planet.party],TREE1(!planet.party),  treeW, treeH);
  }
}

void Game::letCollide(){
  /////////////////
  // CollisionTesting
  ////////////////
  for(size_t party=0; party<PN; party++) {
    size_t rival = !party; // opponents party ID :)
    sShot * shots = mShots[party].shots;
    for(size_t i=0; i<mShots[party].size; i++){
      if(shots[i].timeToLive>0){ // shot exists 
        /////////////////////
        // test against ships
        for(size_t targetId : TREE(rival,(int)shots[i].x/GRID_SIZE, (int)shots[i].y/GRID_SIZE).shiplist){
          sShip & target = mShips[rival].ships[targetId];
          if(target.health && distanceSQ(shots[i].x, shots[i].y, target.x, target.y) < SHIP_RADIUS*SHIP_RADIUS) {
            // collision!!!
            shots[i].timeToLive = -1;
            takeDamage(target);
            break;
          }
        }
        ///////////////////////
        // test against planets
        if(shots[i].timeToLive>0) { // shot did not hit a ship, still alive
          sPlanet * planets = mPlanets.planets;
          for(size_t j=0; j<mPlanets.size; j++){ // take the FIRST planet you can find that is not in our party
            if(planets[j].party != party && distanceSQ(shots[i].x, shots[i].y, planets[j].x, planets[j].y) < PLANET_RADIUS*PLANET_RADIUS) {
              // collision!!!
              shots[i].timeToLive = -1;
              takeDamage(planets[j], party);
              break;
            }
          }
        }
      }
    }
  }
}

void * Game::packData(size_t & size, double time) {
/*
Time double
mMoney float
Number of planets, ships, shots size_t
planets
shipsA
shipsB
shotsA
shotsB
*/
  size_t memShips = sizeof(sShip)*mShips[0].size;
    
  size_t memShots = sizeof(sShot)*mShots[0].size;

  size_t memPlanets = sizeof(sPlanet) * mPlanets.size;
  
  size_t memOther = sizeof(double) + sizeof(float)*2 + sizeof(size_t)*3; // time, 2mMoney, memoryUsage of ship/shot/planets
  
  size = memOther + memPlanets + memShips*2 + memShots*2;
  void * const data = calloc(1, size);
  
  char * dat = (char*)data; // temp pointer
  *(double*)dat = time;                       dat += sizeof(double);
  memcpy(dat, mMoney, 2*sizeof(float));        dat += 2*sizeof(float);
  *(size_t*)dat = memPlanets;           dat += sizeof(size_t);
  *(size_t*)dat = memShips;             dat += sizeof(size_t);
  *(size_t*)dat = memShots;             dat += sizeof(size_t);
  memcpy(dat, mPlanets .planets, memPlanets); dat += memPlanets;
  memcpy(dat, mShips[0].ships,   memShips);   dat += memShips;
  memcpy(dat, mShots[0].shots,   memShots);   dat += memShots;
  memcpy(dat, mShips[1].ships,   memShips);   dat += memShips;
  memcpy(dat, mShots[1].shots,   memShots);   dat += memShots;
  return data;
}
double Game::unpackData(void * const data, size_t size, const double time){
  size_t memShips, memShots, memPlanets;
  
  char * dat = (char*)data; // temp pointer
  double dt = time - *(double*)dat; dat += sizeof(double);
  memcpy(mMoney,dat, 2*sizeof(float));        dat += 2*sizeof(float);
  memPlanets = *(size_t*)dat;           dat += sizeof(size_t);
  memShips   = *(size_t*)dat;           dat += sizeof(size_t);
  memShots   = *(size_t*)dat;           dat += sizeof(size_t);
  memcpy(mPlanets .planets, dat, memPlanets); dat+= memPlanets;
  memcpy(mShips[0].ships,   dat, memShips);   dat+= memShips;
  memcpy(mShots[0].shots,   dat, memShots);   dat+= memShots;
  memcpy(mShips[1].ships,   dat, memShips);   dat+= memShips;
  memcpy(mShots[1].shots,   dat, memShots);   dat+= memShots;
  
  return dt;
}
/**
  Take the changed list of saShips, safe the IDs of changed/deleted ships and copies of changed ships in one memory block.
  Ignore duplicates
  number of dead/changed ships are in the beginning of the memory
  saShips - saShip structure with changed-list and ships

returned buffer:
  size_t numberDeadShips, numberChangedShips
  size_t[] deadShipIds, changedShipIds
  sShip[] changedShips
*/
void *  Game::packChangedShips(Party party, size_t & size){
  sShip * ships = mShips[party].ships;
  const size_t S = mShips[party].changed.size();
  std::sort(mShips[party].changed.begin(), mShips[party].changed.end()); // sort vector to easily ignore dublicates
  size_t pC=0, pD=0; // pointer=amount changed and dead ships
  size_t data[S]; 
  if(!mShips[party].changed.empty()) { // only if changed is not empty
    size_t lastId = mShips[party].changed[0]+1; // this way the first ID cannot be ignored because it cannot equal the "last one"
    for(size_t i=0; i<S; i++){
      size_t curId = mShips[party].changed[i];
      if(curId==lastId){ // skip duplicates
        continue;
      }
      if(ships[curId].health > 0){ // ship changed
        data[pC]=curId;
        pC++;
      } else { // ship is dead
        data[S-1-pD] = curId;
        pD++;
      }
      lastId = curId;
    }  
  }
  size = (pC+pD+2)*sizeof(size_t)+pC*sizeof(sShip); // +2 because of pC and pD
  void * const rdata = calloc(1, size);
  char * dat = (char*)rdata;
  *(size_t*) dat = pD; dat+=sizeof(size_t);
  *(size_t*) dat = pC; dat+=sizeof(size_t);
  memcpy(dat, data+S-pD,pD*sizeof(size_t));   dat+=pD*sizeof(size_t); // ids of dead ships
  memcpy(dat, data,     pC*sizeof(size_t));   dat+=pC*sizeof(size_t); // ids of changed ships
  for(size_t i = 0; i<pC; i++){ // copy changed ships to buffer
    sShip * ship = &ships[data[i]];
    memcpy(dat, ship, sizeof(sShip));
    dat += sizeof(sShip);
  }
  
  // return stuff
  return rdata;
}
void    Game::unpackChangedShips(Party party, void * const data, const double dt){
  char * dat = (char*)data;
  // get numbers
  const size_t deadShips    = *(size_t*)dat;  dat += sizeof(size_t);
  const size_t changedShips = *(size_t*)dat;  dat += sizeof(size_t);
  // get pointer to ID-arrays
  size_t * dead     = (size_t*)dat;           dat += sizeof(size_t) * deadShips;
  size_t * changed  = (size_t*)dat;           dat += sizeof(size_t) * changedShips ;
  // get pointer to ship-data arrays
  sShip * ships = (sShip*)dat;                dat += sizeof(sShip) * changedShips ;
  
  // make dead ships dead :)
  for(size_t i=0; i<deadShips; i++) {
    mShips[party].ships[dead[i]].health = 0;
  }
  // overwrite changed ships and update them to "now"
  for(size_t i=0; i<changedShips; i++) {
    mShips[party].ships[changed[i]] = ships[i]; // overwrite
    updateShip(mShips[party].ships[changed[i]], dt); // update
  }
  
}
/**
returned buffer:
  size_t startID, amount of shots
  sShot... shotdata
*/
void *  Game::packChangedShots(Party party, size_t & size){
  const size_t S = mShots[party].size;
  size_t pStart = mShots[party].changedPos;
  size_t pEnd   = mShots[party].insertPos;
  size_t pLen   = (pEnd-pStart+S)%S; // amount of shots
  
  size = 2*sizeof(size_t)+pLen*sizeof(sShot); // 2 because of pStart and pLen
  
  void * const rdata = calloc(1, size);
  char * dat = (char*)rdata;
  *(size_t*) dat = pStart;  dat+=sizeof(size_t);
  *(size_t*) dat = pLen;    dat+=sizeof(size_t);
  if(pStart+pLen < S) { // consecutive data
    memcpy(dat, mShots[party].shots+pStart,pLen*sizeof(sShot)); dat+=pLen*sizeof(sShot);
  }else { // pStart-S, 0-pEnd
    memcpy(dat, mShots[party].shots+pStart,(S-pStart)*sizeof(sShot)); dat+=(S-pStart)*sizeof(sShot); // pStart-S
    memcpy(dat, mShots[party].shots,pEnd*sizeof(sShot)); dat+=pEnd*sizeof(sShot); // 0-pEnd
  }
  
  return rdata;
}
void    Game::unpackChangedShots(Party party, void * const data, const double dt){
  const size_t S = mShots[party].size;

  char * dat = (char*)data;
  const size_t pStart = *(size_t*) dat;  dat+=sizeof(size_t);
  const size_t pLen   = *(size_t*) dat;  dat+=sizeof(size_t);
  if(pStart+pLen < S) { // consecutive data
    memcpy(mShots[party].shots+pStart, dat, pLen*sizeof(sShot)); dat+=pLen*sizeof(sShot);
  }else { // pStart-S, 0-pEnd
    const size_t pEnd = (pStart+pLen)%S;
    memcpy(mShots[party].shots+pStart, dat, (S-pStart)*sizeof(sShot)); dat+=(S-pStart)*sizeof(sShot); // pStart-S
    memcpy(mShots[party].shots, dat, pEnd*sizeof(sShot)); dat+=pEnd*sizeof(sShot); // 0-pEnd
  }
  // update shots to "now"
  updateShotsIntervall(mShots[party], pStart, pLen, dt);
}
/**
returned buffer
  double time
  float mMoneyA, mMoneyB
  size_t sizeBuf_of_each: planets,ShipsA,shipsB,ShotsA,shotsB
  bufferPlanets
  bufferShips PA
  bufferShips PB
  bufferShots PA
  bufferShots PB
*/
void *  Game::packUpdateData(size_t & size, double time){
  const size_t memPlanets = sizeof(sPlanet) * mPlanets.size; // memory usage for planets
  size = sizeof(double) + 2*sizeof(float) + 5*sizeof(size_t) + memPlanets; // memory usage for ...
  void * data[4];  // pointers to the update-buffers of ships/shots, each per party. shipA/shotA/shipB/shotB
  size_t sizes[4]; // memory usage of ships/shots to update, each per party
  for(size_t party=PA; party<PN; party++){
    data[2*party+0] = packChangedShips((Party)party, sizes[2*party+0]);
    data[2*party+1] = packChangedShots((Party)party, sizes[2*party+1]);
    size += sizes[2*party+0] + sizes[2*party+1]; // += memShipsX + memShotsX
  }
  
  void * rdata = calloc(1, size);
  char * dat = (char*)rdata;
  *(double*)dat = time;                       dat += sizeof(double);    // time
  memcpy(dat, mMoney, 2*sizeof(float));        dat += 2*sizeof(float);   // 2x mMoney
  *(size_t*)dat = memPlanets;                 dat += sizeof(size_t);    // 1x memory for planets
  memcpy(dat, sizes, 4*sizeof(size_t));       dat += 4*sizeof(size_t);  // 4x memory for ship/shot updates
  memcpy(dat, mPlanets .planets, memPlanets); dat += memPlanets;
  for(size_t i=0; i<4; i++){ // copy ships and shots
    memcpy(dat, data[i], sizes[i]); 
    dat+=sizes[i];
    free(data[i]);
  }
  
  return rdata;
}
double  Game::unpackUpdateData(void * const data, size_t size, const double time){
  size_t memPlanets;
  size_t sizes[4];
  
  char * dat = (char*)data; // temp pointer
  const double dt = time - *(double*)dat; dat += sizeof(double);
  memcpy(mMoney,dat, 2*sizeof(float));        dat += 2*sizeof(float); // mMoney
  //get memory usages
  memPlanets = *(size_t*)dat;           dat += sizeof(int);
  memcpy(sizes,dat, 4*sizeof(size_t));        dat += 4*sizeof(size_t); // sizes of shipsa/b, shotsa/b
  // copy actual data
  memcpy(mPlanets .planets, dat, memPlanets); dat+= memPlanets;
  // unpack ships and shots (shipsA,shotsA,shipsB,shotsB)
  for(size_t party=PA; party<PN; party++){
    unpackChangedShips((Party)party, dat,                  dt);
    unpackChangedShots((Party)party, dat+sizes[2*party+0], dt);
    dat += sizes[2*party+0] + sizes[2*party+1]; // += memShipsX + memShotsX
  }
  return dt; 
}

void Game::clearChanged(){
  for(size_t party=PA; party<PN; party++){
    mShips[party].changed.clear();
    mShots[party].changedPos = mShots[party].insertPos;
  }
}

/// get length of vector
inline double Game::vecLen(const vec2 v){
  return sqrt(v.x*v.x + v.y*v.y);
}

/// set dx, dy relative to vector (xy)->(tx,ty)
inline void Game::delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy){
  dx = tx-x;
  dy = ty-y;
}
inline void Game::normalize(float & x, float & y, const float LEN){
  const float normFac = LEN/sqrt(x*x + y*y);
  x = normFac*x;
  y = normFac*y;
  // because of division by 0 we may have some NaNs
  if(std::isnan(x)) x=0;
  if(std::isnan(y)) y=0;
}
inline size_t Game::distanceSQ(const float x, const float y, const float x2, const float y2) {
  return (x-x2)*(x-x2)+(y-y2)*(y-y2);
}
void Game::flyToTarget(sShot & shot, const float tx, const float ty){
  delta(shot.x, shot.y, tx, ty, shot.dx, shot.dy);
  normalize(shot.dx, shot.dy, SHOT_SPEED);
}
void Game::flyToTarget(Party party, const size_t id, const float tx, const float ty){
  saShip & ships = mShips[party];
  sShip & ship = ships.ships[id]; // reference for easy access
  
  // dont let them go out mMap
  ship.tx = (tx<0)?5:(tx>=mMap.w?mMap.w-5:tx); // todo define macro or so to make nicer ...
  ship.ty = (ty<0)?5:(ty>=mMap.h?mMap.h-5:ty); // todo why -5 ?? 

  delta(ship.x, ship.y, ship.tx, ship.ty, ship.dx, ship.dy);
  normalize(ship.dx, ship.dy, SHIP_SPEED);
  
  
  ships.changed.push_back(id); // remember ship as changed
}
void Game::deleteShip(Party party, const size_t id){
  saShip & ships = mShips[party];
  ships.ships[id].health = 0;  // mark ship as dead 
  ships.free[ships.freePush] = id; // insert index to freeIndices list
  ships.freePush = (ships.freePush+1)%ships.size;           // increment insert pointer
  ships.changed.push_back(id); // mark as changed
}

bool Game::addShot(Party party, const float x, const float y, const float tx, const float ty) {
  saShot & shots = mShots[party];
  sShot & shot = shots.shots[shots.insertPos];
  shot.timeToLive = SHOT_LIFETIME;
  shot.x = x;
  shot.y = y;
  
  flyToTarget(shot, tx, ty);
  shots.insertPos = (shots.insertPos+1)%shots.size; // increase insertpointer
  return true;
}

bool Game::addShip(Party party, const float x, const float y, const float tx, const float  ty){
  saShip & ships = mShips[party];
  // Check if the shiplist is full (no elements between freepop and freepush and the ship they are pointing to is alive)
  if(ships.freePop==ships.freePush && ships.ships[ships.free[ships.freePop]].health){
      return false;
  } else {  
    const size_t insertPos = ships.free[ships.freePop];
    sShip & ship = ships.ships[insertPos];
    ship.health = SHIP_HEALTH_MAX;
    ship.timeToShoot = 0;
    ship.x = x;
    ship.y = y;
    flyToTarget(party,insertPos,tx,ty);
    
    ships.freePop = (ships.freePop+1)%ships.size;// increment free index counter
    return true;
  }
}

bool Game::upgradePlanet(sPlanet & planet, Upgrade upgrade) {
  if(planet.level[upgrade] < UPGRADE_MAX_LVL){
    size_t costs = UPGRADE_COSTS; // const upgrade costs
    if(mMoney[planet.party] >= costs) {
      mMoney[planet.party] -= costs;
      planet.level[upgrade]++;
    }
  }
  return false;
}

void Game::capturePlanet(sPlanet & planet, const size_t newParty){
  // When a planet gets neutral, it looses a lot of it's infrastructure
  if(newParty == PN) {
    // ECONOMY is a little bit affected
    planet.level[ECONOMY] = std::max(0, planet.level[ECONOMY]-1);
    // More than half of the DEFENSE is destroyed. 10 -> 5 -> 2 -> 1 -> 0
    planet.level[DEFENSE] = planet.level[DEFENSE]/2;
    // PRODUCTION is a little bit affected
    planet.level[PRODUCTION] = std::max(0, planet.level[ECONOMY]-1);
  }
  // SET new party and reset health
  planet.party = newParty;
  // set health to 100% (remember, neutral full health is 0 ... ;) confusing, huh? But this way we can store it in one var)
  planet.health = (newParty==PN)?0:HEALTH_MAX; 
}

void Game::takeDamage(sShip & ship){
  ship.health--;
  if(ship.health<=0){ // ship dead
    ship.health = -1; // mark for deletion
  }
}
/**
planet - planet to take damage/ that was shot
party - party dealing the damage
*/
void Game::takeDamage(sPlanet & planet, const size_t party){
  if(planet.party == PN){ // neutral planet
    planet.health -= (signed int)party*2-1; // take one for PA, add one for PB. Remember, neutral planets are full health at 0 and overtaken at +-100.
    if(planet.health <= -HEALTH_MAX || planet.health >= HEALTH_MAX){ // overtake
      capturePlanet(planet, party);
    }
  } else { // belongs to the enemy party! 
    if (!planet.shieldActive) {
      planet.health --; // take life away!
      if(planet.health <= 0) { // make neutral!
        capturePlanet(planet, PN);
      }
    }
  }
}
/**
ship/planet - ship/planet to check for closest enemy and shoot
sPlanets - list of planets
shots - list of shots from the same party as ships
rivalTree - spacepartioning tree with the rival-partys ships
W,H - Size of rivalTree
*/
bool Game::shoot(sShip & ship, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const size_t W, const size_t H, Party party){
    const size_t MAX_GRIDS = (2*SHIP_AIM_RANGE/GRID_SIZE+1)*(2*SHIP_AIM_RANGE/GRID_SIZE+1); // at max we need to check this many grids
  
  int dir = 0; // direction we're inserting the next lines in the rectangle
  // location to insert ships (+1 because first dir is up(-1))
  int lx = (int)ship.x/GRID_SIZE;
  int ly = (int)ship.y/GRID_SIZE+1; 
  int stepsPerLevel = 1; // how many ships to draw per line (will be counted down)
  int level = 0; // how many ships per level
  int repeat = 1; // how many lines to draw before level++  (will be counted down)
  
  bool targetFound = false; // will be true when found ;)
  for(size_t j=0; j<MAX_GRIDS && !targetFound; j++) {
    switch(dir){ // get location for next insertion
    case 0:
            ly --;
            break; // up
    case 1:
            lx ++;
            break; // right
    case 2:
            ly ++;
            break; // down
    case 3:
            lx --;
            break; //left
    }
    if(ly >= 0 && (size_t)ly < H && lx >= 0 && (size_t)lx < W && rivalTree[lx*W+ly].size) {           // valid lxy and not empty 
      const Party rival = (Party)(size_t)!party;
      // todo check if square is in range
      // range checking in here :)
      for(size_t targetId : rivalTree[lx*W+ly].shiplist){
        sShip & target = mShips[rival].ships[targetId];
        if(distanceSQ(ship.x, ship.y, target.x, target.y) < SHIP_AIM_RANGE_SQ) {
          // okey we found someone to shoot
          targetFound = true;
          addShot(party, ship.x, ship.y, target.x, target.y);
          ship.timeToShoot += SHIP_SHOOT_DELAY;
          return true;
        }
      }
    }
    
    // further calculations for spiral stuff
    stepsPerLevel--;
    if(stepsPerLevel <= 0){
        dir = (dir + 1) % 4;
        repeat--;
        if(repeat <= 0){
            level++;
            repeat = 2;
        }
        stepsPerLevel = level;
    }
  }
  // aim for planets
  if(!targetFound) {
    for(size_t i=0; i<sPlanets.size; i++){
      sPlanet & target = sPlanets.planets[i];      
      if(target.party == party || target.shieldActive) { // planet from same party or shielded
        continue;
      }
      if(distanceSQ(ship.x, ship.y, target.x, target.y) < SHIP_AIM_RANGE_SQ) {
        // okey we found someone to shoot
        targetFound = true;
        addShot(party, ship.x, ship.y, target.x, target.y);
        ship.timeToShoot += SHIP_SHOOT_DELAY;
        return true;
      }
    }
  }
  
  
  return true; // todo
}
bool Game::shoot(sPlanet & planet, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const size_t W, const size_t H){
  return shoot(*(sShip*)(((double*)&planet)+1), sPlanets, shots, rivalTree, W, H, (Party)planet.party);
}

void Game::updatePlanets(const  double dt){
  sPlanet * planets = (sPlanet*)(const char*)mPlanets.planets;
  for(size_t i=0; i<mPlanets.size; i++){
    //planets[i].tx = mouseV.x; // todo remove
    //planets[i].ty = mouseV.y;
    
    if(planets[i].party!=PN){ // not neutral
      // generate mMoney for the player
      mMoney[planets[i].party] += (planets[i].level[ECONOMY]+1) * MONEY_GEN * dt; 
      // build ships if queue is not empty and production time is over
      if(planets[i].shipQueue) { // ships in queue
        planets[i].timeToBuild -= dt;      // subtract time
        if(planets[i].timeToBuild < 0) { // send a new ship out
          // create new ship 
          // put ship in a random circle around tx/ty
          float dx = rand(-50,50), dy = rand(-50,50);
          float len = sqrt(dx*dx + dy*dy);
          dx = dx*SEND_SHIP_RAND_RADIUS/len;
          dy = dy*SEND_SHIP_RAND_RADIUS/len;
          // add it
          if(!addShip((Party)planets[i].party, planets[i].x,planets[i].y,planets[i].tx+dx,planets[i].ty+dy)) {
            fprintf(stderr, "ship insert failed\n"); // todo think of sth better than restoring mMoney
            // restore mMoney to player
            mMoney[planets[i].party] += SHIP_COSTS * planets[i].shipQueue;
            planets[i].shipQueue = 0; 
          } else {
            planets[i].shipQueue --; // decrement build list
          }
          
          // reset timeToBuild 
          planets[i].timeToBuild += SHIP_PROD_TIME*(1-(planets[i].level[PRODUCTION]+1)/UPGRADE_MAX_LVL);
        }
      }
      // use power if shield if active
      if(planets[i].shieldActive) {
        planets[i].power -= dt*(double)POWER_DRAIN/(planets[i].level[DEFENSE]+1);
        if(planets[i].power<0) //  no more power -> turn shield off
          planets[i].shieldActive = false;
      } else { // restore power, shield is off
        planets[i].power += POWER_REGEN*(planets[i].level[DEFENSE]+1)*dt;
        if(planets[i].power > POWER_MAX) // limit to maximum powercapacity
          planets[i].power = POWER_MAX;
      }
    }
    // restore health
    if(planets[i].party == PN) {                // neutral planets are special, 0=full health. The sign tells us who is trying to take it over
      if(planets[i].health < 0) {               // PA is trying to take it
        planets[i].health += HEALTH_REGEN * dt; // regenerate
        if(planets[i].health > 0) {
          planets[i].health = 0;                // set to full health
        }     
      } else if (planets[i].health > 0) {       // PB is trying to take it
        planets[i].health -= HEALTH_REGEN * dt; // regenerate
        if(planets[i].health < 0) {
          planets[i].health = 0;                // set to full health
        }
      }
    } else  {                                    // Planet owned by a party, they act normal xD
      planets[i].health += HEALTH_REGEN * dt;   // regenerate
      if(planets[i].health > HEALTH_MAX)
        planets[i].health = HEALTH_MAX;         // set to full health
    } /**/
    // reload weapons
    planets[i].timeToShoot -= dt * (1+(float)planets[i].level[DEFENSE]/4);
  }
}

void Game::updateShotsIntervall(saShot & sShots, const size_t pStart, const size_t num, const double dt){
  if( pStart+num > sShots.size ){ // shots to update overlap end
    updateShotsIntervall(sShots, pStart, sShots.size-pStart,       dt);  // pStart-END
    updateShotsIntervall(sShots, 0,      (num+pStart)%sShots.size, dt);  // BEGIN-x
  } else {
    sShot * shots = sShots.shots;
    for(size_t i=pStart; i<pStart+num && i<sShots.size; i++){
      if(shots[i].timeToLive>0){ // shot exists
        // decrease lifetime
        shots[i].timeToLive -= dt;
        // move shot
        shots[i].x += shots[i].dx * dt;
        shots[i].y += shots[i].dy * dt;
        // shots outside screen will be deleted
        if(shots[i].x<0 || shots[i].x>mMap.w || shots[i].y<0 || shots[i].y>mMap.h) {
          shots[i].timeToLive = -1;
        }
      }
    }
  }
}

void Game::updateShots(const double dt){
  for(size_t party=0; party<PN; party++) {
    updateShotsIntervall(mShots[party],500, mShots[party].size, dt);
  }
}

void Game::updateShip(sShip & ship, const double dt){
  if(ship.health){ // ship alive, handle it
    // if moving, move :)
    if(ship.dx || ship.dy) {
      // move
      ship.x += ship.dx *dt;
      ship.y += ship.dy *dt;
      // if overshooting target, teleport to target
      float dx = ship.tx - ship.x;
      float dy = ship.ty - ship.y;
      if((dx>0 && ship.dx<0)||(dx<0 && ship.dx>0)) {ship.x=ship.tx; ship.dx=0;}
      if((dy>0 && ship.dy<0)||(dy<0 && ship.dy>0)) {ship.y=ship.ty; ship.dy=0;}
      
    }
    // reduce shooting time
    ship.timeToShoot -= dt;  
  }
}
void Game::updateShips(const double dt){
  for(size_t party=0; party<PN; party++) {
    sShip * ships = mShips[party].ships;
    for(size_t i=0; i<mShips[party].size; i++){
      if(ships[i].health < 0){
        deleteShip((Party)party, i);
      } else {
        updateShip(ships[i],dt);
      }
    }
  }
}

void Game::initGame(saPlanet & planets, saShip * ships, saShot * shots, const size_t MAX_SHIPS) {
  initPlanets(planets, 6);
  const size_t MAX_SHOTS = MAX_SHIPS*(float)SHOT_LIFETIME/(float)SHIP_SHOOT_DELAY + 1000; // + 1000 just to be sure
  initShots(shots[PA],  MAX_SHOTS);
  initShots(shots[PB],  MAX_SHOTS);
  initShips(ships[PA],  MAX_SHIPS);
  initShips(ships[PB],  MAX_SHIPS);
}
void Game::initPlanets(saPlanet & planets, const size_t size){
  planets.size = size;
  planets.planets = new sPlanet[planets.size];
  //memset(planets.planets, 0, sizeof(sPlanet)*size); // clear
  
  planets.planets[0] = sPlanet{0,0,100,100,400,125,0,9,0, PA,3000,80,5,true};
  planets.planets[1] = sPlanet{0,0,100,250,400,275,0,9,0, PA,3000,80,5,true};
  planets.planets[2] = sPlanet{0,0,100,400,400,425,0,9,0, PA,3000,80,5,true};
  planets.planets[3] = sPlanet{0,0,700,100,400, 75,0,9,0, PB,3000,80,5,true};
  planets.planets[4] = sPlanet{0,0,700,250,400,225,0,9,0, PB,3000,80,5,true};
  planets.planets[5] = sPlanet{0,0,700,400,400,375,0,9,0, PB,3000,80,5,false};
}
void Game::initShots(saShot & shots, const size_t size){
  shots.size = size;
  shots.insertPos = 0;
  shots.changedPos = 0;
  shots.shots = new sShot[shots.size];
  memset(shots.shots, 0, sizeof(sShot)*size); // clear data
}
void Game::initShips(saShip & ships, const size_t size){
  ships.size = size;
  ships.ships = new sShip[ships.size];
  
  memset(ships.ships, 0, sizeof(sShip)*size); // clear
  ships.freePush = 0;
  ships.freePop  = 0;
  ships.free = new size_t[ships.size];
  for(size_t i=0; i<ships.size; i++)
    ships.free[i]=i;
  
}

Game::GameConfig Game::getConfig(){
  
  
  return config;
}

void Game::select(Party party, vec2 v){
  // todo implement select(vec2) to select one ship or one planet
  vec2 u = {v.x-SHIP_RADIUS, v.y-SHIP_RADIUS};
  v = {v.x+SHIP_RADIUS, v.y+SHIP_RADIUS};
  select(party, u,v); // right now it's indirectly just deselecting
}
/** selects ships within the rectangle formed by the two points v1, v2 
the selected ships can be found in the list Game::selectedShips
*/
void Game::select(Party party, vec2 v1, vec2 v2){
  if(mTree == nullptr)
    return;
  // todo: select depending on which party the player is on PA/PB
  // todo: work with mods(keyboard modifiern like shift/ctrl to select/unselect ships with this function!)
  
  vec2 va{std::min(v1.x, v2.x), std::min(v1.y, v2.y)}; // upper left
  vec2 vb{std::max(v1.x, v2.x), std::max(v1.y, v2.y)}; // lower right
  
  // positions in space partitioning tree: 
  vec2 const gridA{(float)((int)va.x/GRID_SIZE),(float)((int)va.y/GRID_SIZE)};
  vec2 const gridB{(float)((int)vb.x/GRID_SIZE),(float)((int)vb.y/GRID_SIZE)};
  printf("selecting[ %f/%f %f/%f\n",v1.x,v1.y,v2.x,v2.y);
  printf("selecting- %f/%f %f/%f\n",va.x,va.y,vb.x,vb.y);
  printf("selecting] %f/%f %f/%f\n",gridA.x,gridA.y,gridB.x,gridB.y);

  // empty selected Ship list
  selectedShips.clear();
  // traverse tree to look for ships within this range
  for(size_t x=gridA.x; x<=gridB.x; x++){
    for(size_t y=gridA.y; y<=gridB.y; y++){
      for(size_t targetId : TREE(party,x,y).shiplist){
        sShip & target = mShips[party].ships[targetId];
        if(target.x >= va.x && target.x <= vb.x && target.y >= va.y && target.y <= vb.y) { // within rectangle
          selectedShips.push_back(targetId); // mark as selected
        }
      }
    }
  }
  
  printf("selected %d ships\n",selectedShips.size());
}


/** get data to send to server to ask him to send the ships to the desired place
v1 start click/drag
v2 end click/drag coords
formation is formation to use // todo


size_t numberOfElements
size_t[] elementIds
vec2[] newPositions
*/
void * Game::sendSelectedGetData(Party party, vec2 v1, vec2 v2, size_t formation, size_t & size){
  std::sort(selectedShips.begin(), selectedShips.end()); // todo necessary?
  
  // remove dead ships
  selectedShips.erase(std::remove_if(selectedShips.begin(),selectedShips.end(),[&](size_t i){return !(mShips[party].ships[i].health>0);}), selectedShips.end()); 
  
  printf("prep send data: size=%d\n", selectedShips.size());
  std::vector<vec2> targetPositions; //todo init with same size as selectedShips
  
  switch(formation){
    case 0: // square
    {
      const size_t S = selectedShips.size();
      const vec2 mouseVec = vec2{v2.x-v1.x, v2.y-v1.y};
      const float mouseDelta = vecLen(vec2{mouseVec.x, mouseVec.y});
      const float offset = std::max(mouseDelta/10, (float)SHIP_RADIUS); // min-offset of SHIP_RADIUS
      
      int dir = 0; // direction we're inserting the next lines in the rectangle
      int lx = 0, ly = 1; // location to insert ships (+10 because first dir is up(-10))
      int shipsPerLevel = 1; // how many ships to draw per line (will be counted down)
      int level = 0; // how many ships per level
      int repeat = 1; // how many lines to draw before level++  (will be counted down)

      for(size_t i : selectedShips){ 
          switch(dir){ // get location for next insertion
          case 0:
                  ly --;
                  break; // up
          case 1:
                  lx ++;
                  break; // right
          case 2:
                  ly ++;
                  break; // down
          case 3:
                  lx --;
                  break; //left
          }

          targetPositions.push_back(vec2{v1.x+lx*offset, v1.y+ly*offset});//(vec2{v1.x+lx*offset, v1.y+ly*offset});

          shipsPerLevel--;
          if(shipsPerLevel <= 0){
              dir = (dir + 1) % 4;
              repeat--;
              if(repeat <= 0){
                  level++;
                  repeat = 2;
              }
              shipsPerLevel = level;
          }
      }
    }break;
    case 3: // relative move, ships keep distance relative to middle of formation
    {
      // calculate scaling
      const float mouseDelta = vecLen(vec2{v2.x-v1.x, v2.y-v1.y});
      float scale;
      if(mouseDelta<10)
        scale = 1;
      else
        scale = mouseDelta/100;// 100pix=keep scale
      // find middle coords of formation
      const size_t S = selectedShips.size();
      vec2 middle{0,0};
      for(size_t i : selectedShips){  
        middle.x += mShips[party].ships[i].x;
        middle.y += mShips[party].ships[i].y;
      }
      middle = vec2{middle.x/S, middle.y/S}; // middlepoint of all ships
      // move ships relative
      for(size_t i : selectedShips){ 
        const vec2 vOld = vec2{mShips[party].ships[i].x, mShips[party].ships[i].y};
        vec2 vDv = vec2{vOld.x - middle.x, vOld.y - middle.y}; // shipXY to middle
        vDv = vec2{vDv.x*scale, vDv.y*scale}; // scale
        
        // vNew = (vOld-middle)*scale
        const vec2 vNew = vec2{v1.x + vDv.x, v1.y + vDv.y};
        targetPositions.push_back(vNew);    
      }
    }break;
    case 1: // line
    {
      const size_t S = selectedShips.size();
      const vec2 lineVec = vec2{v2.x-v1.x, v2.y-v1.y};
      vec2 lineDelta = vec2{lineVec.x/S, lineVec.y/S};
      if(vecLen(lineDelta) < SHIP_RADIUS) { // minimum offset of SHIP_RADIUS todo tweak
        normalize(lineDelta.x, lineDelta.y, SHIP_RADIUS);
      }
      
      vec2 linePos = v1;
      for(size_t i : selectedShips){          
        targetPositions.push_back(linePos);    
        linePos = vec2{linePos.x+lineDelta.x, linePos.y+lineDelta.y};
      }
    }break;
    case 2: // circle
    { // todo minimum size, maybe depending on amount of ships
      // todo std size if radius=0 (can be included in todo above)
      const float radius = vecLen(vec2{v2.x-v1.x, v2.y-v1.y});
      for(size_t i : selectedShips){          
        vec2 dv{randf(), randf()}; 
        normalize(dv.x, dv.y, radius);
        targetPositions.push_back(vec2{v1.x+dv.x, v1.y+dv.y});    
      }
    }break;
  }
  
  size = sizeof(size_t)+(sizeof(size_t)+sizeof(vec2))*selectedShips.size();
  void * const data = calloc(size, 1);
  char * dat = (char*)data;
  *(size_t*) dat = selectedShips.size(); // store amount
  printf("prep send data: stored size=%d\n", *(size_t*) dat);
  dat += sizeof(size_t);
  memcpy(dat, &selectedShips[0], sizeof(size_t)*selectedShips.size());// ids
  dat += sizeof(size_t)*selectedShips.size();
  memcpy(dat, &targetPositions[0], sizeof(vec2)*targetPositions.size());//vecs
  
  return data;
}

/**
 take data from above and execute on ships
*/
void Game::sendShips(Party party, void * const data){
  char * const dat = (char *)data;
  const size_t S = *(size_t*)dat;
  size_t * ids = (size_t*)(dat+sizeof(size_t));
  vec2 * vecs = (vec2*)(dat+sizeof(size_t)+sizeof(size_t)*S);
  for(size_t i=0; i<S; i++){
    flyToTarget(party, ids[i], vecs[i].x, vecs[i].y);
  }
}


    
