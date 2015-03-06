#include <algorithm>
#include "game.hpp"
#define TREE(X,Y,Z)  mTree[(X*treeW+Y)*treeH+Z] // from XYZ to [x][y][z]
#define TREE1(X)  &mTree[X*treeW*treeH] // from XYZ to [x][y][z]

float money[PN] = {0,0}; // money of PA and PB

/// init list of game objects
void initGame(saPlanet & planets, saShip * ships, saShot * shots, const unsigned int MAX_SHIPS);
void initPlanets(saPlanet & planets, const unsigned int size);
void initShips(saShip & ships, const unsigned int size);
void initShots(saShot & shots, const unsigned int size);
/// process list of game objects
void updatePlanets(saPlanet & sPlanets, saShip * sShips, const double dt);
void updateShips(saShip * sShips, const double dt);
void updateShots(saShot * sShots, const double dt);
void updateShip(sShip & ship, const double dt); // single ship update

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


/// helping functions
inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy); /// dx=tx-x
inline void normalize(float & x, float & y, const float LEN);  /// normalized vector * LEN
inline unsigned int distanceSQ(const float x, const float y, const float x2, const float y2); /// distance^2
/// command ship/shot to go somewhere
void flyToTarget(sShot & shot, const float tx, const float ty);
void flyToTarget(saShip & ships, const unsigned int id, const float tx, const float ty);
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

Game::Game(const unsigned int MAX_SHIPS, const unsigned int NUM_PLANETS){
  
  treeW = map.w/GRID_SIZE;
  treeH = map.h/GRID_SIZE;
  
  const unsigned int MAX_SHOTS = MAX_SHIPS*(float)SHOT_LIFETIME/(float)SHIP_SHOOT_DELAY + 1000; // + 1000 just to be sure
  initPlanets(mPlanets,  NUM_PLANETS);
  initShots(mShots[PA],  MAX_SHOTS);
  initShots(mShots[PB],  MAX_SHOTS);
  initShips(mShips[PA],  MAX_SHIPS);
  initShips(mShips[PB],  MAX_SHIPS);  
}

void Game::update(const double dt){
  updatePlanets(mPlanets, mShips, dt);
  updateShips(mShips, dt);
  updateShots(mShots, dt);
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
  for(unsigned int party=PA; party<PN; party++)
    for(unsigned int x=0; x<treeW; x++)
      for(unsigned int y=0; y<treeH; y++){
        TREE(party,x,y).size = 0;
        TREE(party,x,y).shiplist.clear();
      }
  
  /// Fill space partitioning structure with ships
  // todo try to optimize, takes 100fps away
  for(unsigned int party=PA; party<PN; party++){
    sShip * ships = mShips[party].ships;
    for(unsigned int i=0; i<mShips[party].size; i++){
      if(ships[i].health){
        TREE(party,(unsigned int)(ships[i].x)/GRID_SIZE,(unsigned int)(ships[i].y)/GRID_SIZE).size++;
        TREE(party, (unsigned int)(ships[i].x)/GRID_SIZE, (unsigned int)(ships[i].y)/GRID_SIZE).shiplist.push_front(&ships[i]);
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
  for(unsigned int party=PA; party<PN; party++){
    //sShip * lastTarget = nullptr;
    //float lastTargetDistanceSQ = 0;
    unsigned int rival = !party; // opponents party ID :)
    // let ships shoot
    for(unsigned int i=0; i<mShips[party].size; i++){
      sShip & ship = mShips[party].ships[i];
      if(!ship.health || ship.timeToShoot>0) {// dead or weapon not ready
        continue;
      }
      shoot(ship, mPlanets, mShots[party],TREE1(rival),  treeW, treeH, party);
    }
  }
  // let planets shoot
  for(unsigned int i=0; i<mPlanets.size; i++){
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
  for(unsigned int party=0; party<PN; party++) {
    unsigned int rival = !party; // opponents party ID :)
    sShot * shots = mShots[party].shots;
    for(unsigned int i=0; i<mShots[party].size; i++){
      if(shots[i].timeToLive>0){ // shot exists 
        /////////////////////
        // test against ships
        for(sShip * target : TREE(rival,(int)shots[i].x/GRID_SIZE, (int)shots[i].y/GRID_SIZE).shiplist){
          if(target->health && distanceSQ(shots[i].x, shots[i].y, target->x, target->y) < SHIP_RADIUS*SHIP_RADIUS) {
            // collision!!!
            shots[i].timeToLive = -1;
            takeDamage(*target);
            break;
          }
        }
        ///////////////////////
        // test against planets
        if(shots[i].timeToLive>0) { // shot did not hit a ship, still alive
          sPlanet * planets = mPlanets.planets;
          for(unsigned int j=0; j<mPlanets.size; j++){ // take the FIRST planet you can find that is not in our party
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
Money float
Number of planets, ships, shots unsigned int
planets
shipsA
shipsB
shotsA
shotsB
*/
  unsigned int memShips = sizeof(sShip)*mShips[0].size;
    
  unsigned int memShots = sizeof(sShot)*mShots[0].size;

  unsigned int memPlanets = sizeof(sPlanet) * mPlanets.size;
  
  unsigned int memOther = sizeof(double) + sizeof(float)*2 + sizeof(unsigned int)*3; // time, 2money, memoryUsage of ship/shot/planets
  
  size = memOther + memPlanets + memShips*2 + memShots*2;
  void * const data = calloc(1, size);
  
  char * dat = (char*)data; // temp pointer
  *(double*)dat = time;                       dat += sizeof(double);
  memcpy(dat, money, 2*sizeof(float));        dat += 2*sizeof(float);
  *(unsigned int*)dat = memPlanets;           dat += sizeof(int);
  *(unsigned int*)dat = memShips;             dat += sizeof(int);
  *(unsigned int*)dat = memShots;             dat += sizeof(int);
  memcpy(dat, mPlanets .planets, memPlanets); dat += memPlanets;
  memcpy(dat, mShips[0].ships,   memShips);   dat += memShips;
  memcpy(dat, mShots[0].shots,   memShots);   dat += memShots;
  memcpy(dat, mShips[1].ships,   memShips);   dat += memShips;
  memcpy(dat, mShots[1].shots,   memShots);   dat += memShots;
  return data;
}
double Game::unpackData(void * const data, size_t size, const double time){
  unsigned int memShips, memShots, memPlanets;
  
  char * dat = (char*)data; // temp pointer
  double dt = time - *(double*)dat; dat += sizeof(double);
  memcpy(money,dat, 2*sizeof(float));        dat += 2*sizeof(float);
  memPlanets = *(unsigned int*)dat;           dat += sizeof(int);
  memShips   = *(unsigned int*)dat;           dat += sizeof(int);
  memShots   = *(unsigned int*)dat;           dat += sizeof(int);
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
void * packChangedShips(saShip & saShips, size_t & size){
  sShip * ships = saShips.ships;
  const size_t S = saShips.changed.size();
  std::sort(saShips.changed.begin(), saShips.changed.end()); // sort vector to easily ignore dublicates
  size_t pC=0, pD=0; // pointer=amount changed and dead ships
  size_t data[S]; 
  size_t lastId = saShips.changed[0]+1; // this way the first ID cannot be ignored because it cannot equal the "last one"
  for(size_t i=0; i<S; i++){
    size_t curId = saShips.changed[i];
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
void unpackChangedShips(saShip & saShips, void * const data, const double dt){
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
    saShips.ships[dead[i]].health = 0;
  }
  // overwrite changed ships and update them to "now"
  for(size_t i=0; i<changedShips; i++) {
    saShips.ships[changed[i]] = ships[i]; // overwrite
    updateShip(saShips.ships[changed[i]], dt); // update
  }
  
}
/**
returned buffer:
  size_t startID, stopID
  sShot... shotdata
*/
void * packChangedShots(saShot & saShots, size_t & size){
  const size_t S = saShots.size;
  size_t pStart = saShots.changedPos;
  size_t pEnd   = saShots.insertPos;
  size_t pLen   = (pEnd-pStart+S)%S; // amount of shots
  
  size = 2*sizeof(size_t)+pLen*sizeof(sShot); // 2 because of pStart and pLen
  
  void * const rdata = calloc(1, size);
  char * dat = (char*)rdata;
  *(size_t*) dat = pStart;  dat+=sizeof(size_t);
  *(size_t*) dat = pLen;    dat+=sizeof(size_t);
  if(pStart+pLen < S) { // consecutive data
    memcpy(dat, saShots.shots+pStart,pLen*sizeof(sShot)); dat+=pLen*sizeof(sShot);
  }else { // pStart-S, 0-pEnd
    memcpy(dat, saShots.shots+pStart,(S-pStart)*sizeof(sShot)); dat+=(S-pStart)*sizeof(sShot); // pStart-S
    memcpy(dat, saShots.shots,pEnd*sizeof(sShot)); dat+=pEnd*sizeof(sShot); // 0-pEnd
  }
  
  return rdata;
}
/**
returned buffer
  double time
  float moneyA, moneyB
  size_t sizeBuf_of_each: planets,ShipsA,shipsB,ShotsA,shotsB
  bufferPlanets
  bufferShips PA
  bufferShips PB
  bufferShots PA
  bufferShots PB
*/
void * Game::packUpdateData(size_t & size, double time){
  const size_t memPlanets = sizeof(sPlanet) * mPlanets.size; // memory usage for planets
  size = sizeof(double) + 2*sizeof(float) + 5*sizeof(size_t) + memPlanets; // memory usage for ...
  void * data[4];  // pointers to the update-buffers of ships/shots, each per party. shipA/shipB/shotA/shotB
  size_t sizes[4]; // memory usage of ships/shots to update, each per party
  for(unsigned int party=PA; party<PN; party++){
    data[0+party] = packChangedShips(mShips[party], sizes[0+party]);
    data[2+party] = packChangedShots(mShots[party], sizes[2+party]);
    size += sizes[0+party] + sizes[2+party]; // += memShipsX + memShotsX
  }
  
  void * rdata = calloc(1, size);
  char * dat = (char*)rdata;
  *(double*)dat = time;                       dat += sizeof(double);    // time
  memcpy(dat, money, 2*sizeof(float));        dat += 2*sizeof(float);   // 2x money
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
double Game::unpackUpdateData(void * const data, size_t size, const double time){
  size_t memPlanets;
  size_t sizes[4];
  
  char * dat = (char*)data; // temp pointer
  const double dt = time - *(double*)dat; dat += sizeof(double);
  memcpy(money,dat, 2*sizeof(float));        dat += 2*sizeof(float); // money
  //get memory usages
  memPlanets = *(unsigned int*)dat;           dat += sizeof(int);
  memcpy(sizes,dat, 4*sizeof(size_t));        dat += 4*sizeof(size_t); // sizes of shipsa/b, shotsa/b
  // copy actual data
  memcpy(mPlanets .planets, dat, memPlanets); dat+= memPlanets;
  return dt; // todo
}

void Game::clearChanged(){
  for(unsigned int party=PA; party<PN; party++){
    mShips[party].changed.clear();
    mShots[party].changedPos = mShots[party].insertPos;
  }
}

/// set dx, dy relative to vector (xy)->(tx,ty)
inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy){
  dx = tx-x;
  dy = ty-y;
}
inline void normalize(float & x, float & y, const float LEN){
  const float normFac = LEN/sqrt(x*x + y*y);
  if(std::isnan(normFac)) {
    x=0; 
    y=0; 
  } else {
    x = normFac*x;
    y = normFac*y;
  }
}
inline unsigned int distanceSQ(const float x, const float y, const float x2, const float y2) {
  return (x-x2)*(x-x2)+(y-y2)*(y-y2);
}
void flyToTarget(sShot & shot, const float tx, const float ty){
  delta(shot.x, shot.y, tx, ty, shot.dx, shot.dy);
  normalize(shot.dx, shot.dy, SHOT_SPEED);
}
void flyToTarget(saShip & ships, const unsigned int id, const float tx, const float ty){
  sShip & ship = ships.ships[id]; // reference for easy access
  
  // dont let them go out map
  ship.tx = (tx<0)?0:(tx>=map.w?map.w-5:tx); // todo define macro or so to make nicer ...
  ship.ty = (ty<0)?0:(ty>=map.h?map.h-5:ty); // todo why -5 ?? 

  delta(ship.x, ship.y, ship.tx, ship.ty, ship.dx, ship.dy);
  normalize(ship.dx, ship.dy, SHIP_SPEED);
  
  
  ships.changed.push_back(id); // remember ship as changed
}
void deleteShip(saShip & ships, const unsigned int id){
  ships.ships[id].health = 0;  // mark ship as dead 
  ships.free[ships.freePush] = id; // insert index to freeIndices list
  ships.freePush = (ships.freePush+1)%ships.size;           // increment insert pointer
  ships.changed.push_back(id); // mark as changed
}

bool addShot(saShot & shots, const float x, const float y, const float tx, const float ty) {
  sShot & shot = shots.shots[shots.insertPos];
  shot.timeToLive = SHOT_LIFETIME;
  shot.x = x;
  shot.y = y;
  
  flyToTarget(shot, tx, ty);
  shots.insertPos = (shots.insertPos+1)%shots.size; // increase insertpointer
  return true;
}

bool addShip(saShip & ships, const float x, const float y, const float tx, const float  ty){
  // Check if the shiplist is full (no elements between freepop and freepush and the ship they are pointing to is alive)
  if(ships.freePop==ships.freePush && ships.ships[ships.free[ships.freePop]].health){
      return false;
  } else {  
    const unsigned int insertPos = ships.free[ships.freePop];
    sShip & ship = ships.ships[insertPos];
    ship.health = SHIP_HEALTH_MAX;
    ship.timeToShoot = 0;
    ship.x = x;
    ship.y = y;
    flyToTarget(ships,insertPos,tx,ty);
    
    ships.freePop = (ships.freePop+1)%ships.size;// increment free index counter
    return true;
  }
}

bool upgradePlanet(sPlanet & planet, Upgrade upgrade) {
  if(planet.level[upgrade] < UPGRADE_MAX_LVL){
    unsigned int costs = UPGRADE_COSTS; // const upgrade costs
    if(money[planet.party] >= costs) {
      money[planet.party] -= costs;
      planet.level[upgrade]++;
    }
  }
  return false;
}

void capturePlanet(sPlanet & planet, const unsigned int newParty){
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

void takeDamage(sShip & ship){
  ship.health--;
  if(ship.health<=0){ // ship dead
    ship.health = -1; // mark for deletion
  }
}
/**
planet - planet to take damage/ that was shot
party - party dealing the damage
*/
void takeDamage(sPlanet & planet, const unsigned int party){
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
bool shoot(sShip & ship, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H, const unsigned int PARTY){
    const unsigned int MAX_GRIDS = (2*SHIP_AIM_RANGE/GRID_SIZE+1)*(2*SHIP_AIM_RANGE/GRID_SIZE+1); // at max we need to check this many grids
  
  int dir = 0; // direction we're inserting the next lines in the rectangle
  // location to insert ships (+1 because first dir is up(-1))
  int lx = (int)ship.x/GRID_SIZE;
  int ly = (int)ship.y/GRID_SIZE+1; 
  int stepsPerLevel = 1; // how many ships to draw per line (will be counted down)
  int level = 0; // how many ships per level
  int repeat = 1; // how many lines to draw before level++  (will be counted down)
  
  bool targetFound = false; // will be true when found ;)
  for(unsigned int j=0; j<MAX_GRIDS && !targetFound; j++) {
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
    if(ly >= 0 && (unsigned int)ly < H && lx >= 0 && (unsigned int)lx < W && rivalTree[lx*W+ly].size) {           // valid lxy and not empty 
      // todo check if square is in range
      // range checking in here :)
      for(sShip * target : rivalTree[lx*W+ly].shiplist){
        if(distanceSQ(ship.x, ship.y, target->x, target->y) < SHIP_AIM_RANGE_SQ) {
          // okey we found someone to shoot
          targetFound = true;
          addShot(shots, ship.x, ship.y, target->x, target->y);
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
    for(unsigned int i=0; i<sPlanets.size; i++){
      sPlanet & target = sPlanets.planets[i];      
      if(target.party == PARTY || target.shieldActive) { // planet from same party or shielded
        continue;
      }
      if(distanceSQ(ship.x, ship.y, target.x, target.y) < SHIP_AIM_RANGE_SQ) {
        // okey we found someone to shoot
        targetFound = true;
        addShot(shots, ship.x, ship.y, target.x, target.y);
        ship.timeToShoot += SHIP_SHOOT_DELAY;
        return true;
      }
    }
  }
  
  
  return true; // todo
}
bool shoot(sPlanet & planet, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H){
  return shoot(*(sShip*)(((double*)&planet)+1), sPlanets, shots, rivalTree, W, H, planet.party);
}

void updatePlanets(saPlanet & sPlanets, saShip * sShips, const  double dt){
  sPlanet * planets = (sPlanet*)(const char*)sPlanets.planets;
  for(unsigned int i=0; i<sPlanets.size; i++){
    //planets[i].tx = mouseV.x; // todo remove
    //planets[i].ty = mouseV.y;
    
    if(planets[i].party!=PN){ // not neutral
      // generate money for the player
      money[planets[i].party] += (planets[i].level[ECONOMY]+1) * MONEY_GEN * dt; 
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
          if(!addShip(sShips[planets[i].party], planets[i].x,planets[i].y,planets[i].tx+dx,planets[i].ty+dy)) {
            fprintf(stderr, "ship insert failed\n"); // todo think of sth better than restoring money
            // restore money to player
            money[planets[i].party] += SHIP_COSTS * planets[i].shipQueue;
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

void updateShotsIntervall(saShot & sShots, const unsigned int pStart, const unsigned int num, const double dt){
  if( pStart+num > sShots.size ){ // shots to update overlap end
    updateShotsIntervall(sShots, pStart, sShots.size-pStart,       dt);  // pStart-END
    updateShotsIntervall(sShots, 0,      (num+pStart)%sShots.size, dt);  // BEGIN-x
  } else {
    sShot * shots = sShots.shots;
    for(unsigned int i=pStart; i<pStart+num && i<sShots.size; i++){
      if(shots[i].timeToLive>0){ // shot exists
        // decrease lifetime
        shots[i].timeToLive -= dt;
        // move shot
        shots[i].x += shots[i].dx * dt;
        shots[i].y += shots[i].dy * dt;
        // shots outside screen will be deleted
        if(shots[i].x<0 || shots[i].x>map.w || shots[i].y<0 || shots[i].y>map.h) {
          shots[i].timeToLive = -1;
        }
      }
    }
  }
}

void updateShots(saShot * sShots, const double dt){
  for(unsigned int party=0; party<PN; party++) {
    updateShotsIntervall(sShots[party],500, sShots[party].size, dt);
  }
}

void updateShip(sShip & ship, const double dt){
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
void updateShips(saShip * sShips, const double dt){
  for(unsigned int party=0; party<PN; party++) {
    sShip * ships = sShips[party].ships;
    for(unsigned int i=0; i<sShips[party].size; i++){
      if(ships[i].health < 0){
        deleteShip(sShips[party], i);
      } else {
        updateShip(ships[i],dt);
      }
    }
  }
}

void initGame(saPlanet & planets, saShip * ships, saShot * shots, const unsigned int MAX_SHIPS) {
  initPlanets(planets, 6);
  const unsigned int MAX_SHOTS = MAX_SHIPS*(float)SHOT_LIFETIME/(float)SHIP_SHOOT_DELAY + 1000; // + 1000 just to be sure
  initShots(shots[PA],  MAX_SHOTS);
  initShots(shots[PB],  MAX_SHOTS);
  initShips(ships[PA],  MAX_SHIPS);
  initShips(ships[PB],  MAX_SHIPS);
}
void initPlanets(saPlanet & planets, const unsigned int size){
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
void initShots(saShot & shots, const unsigned int size){
  shots.size = size;
  shots.insertPos = 0;
  shots.changedPos = 0;
  shots.shots = new sShot[shots.size];
  memset(shots.shots, 0, sizeof(sShot)*size); // clear data
}
void initShips(saShip & ships, const unsigned int size){
  ships.size = size;
  ships.ships = new sShip[ships.size];
  
  memset(ships.ships, 0, sizeof(sShip)*size); // clear
  ships.freePush = 0;
  ships.freePop  = 0;
  ships.free = new unsigned int[ships.size];
  for(unsigned int i=0; i<ships.size; i++)
    ships.free[i]=i;
  
}

