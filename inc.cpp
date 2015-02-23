#include "inc.hpp"
struct sInfo info;
struct vec2 map, screen, view, mouseR, mouseV;
const char * textureNames[] = {"mFont.tga","planet.tga","ship.png"};



void drawTree(sSquare* tree, const float dX, const float dY){
  // draw lines
  const unsigned int W=map.w/GRID_SIZE;
  const unsigned int H=map.h/GRID_SIZE;
  const unsigned int WH=W*H;
  
  glBegin(GL_LINES);
  glColor3ub(255,255,0);
  for(unsigned int x=0; x<W; x++){
    for(unsigned int y=0; y<H; y++){
        // vertical
        glVertex2i(dX+x*GRID_SIZE,   dY+y*GRID_SIZE);
        glVertex2i(dX+x*GRID_SIZE,   dY+(y+1)*GRID_SIZE);
        // horizontal
        glVertex2i(dX+x*GRID_SIZE,    dY+y*GRID_SIZE);
        glVertex2i(dX+(x+1)*GRID_SIZE,dY+y*GRID_SIZE);
      
    }
  }
  glEnd();
  // data
  int dx = (int)dX;
  int dy = (int)dY;
  for(unsigned int party=PA; party<PN; party++){
    for(unsigned int x=0; x<W; x++){
      for(unsigned int y=0; y<H; y++){
        glColor3ub(party*255,255-party*255,0);
        drawInt(tree[party*WH+x*H+y].size,dx+x*GRID_SIZE,   dy+y*GRID_SIZE+party*15);
      }
    }
  }
  
}
void drawPlanets(const saPlanet & sPlanets, const float dX, const float dY){  
  sPlanet * planets = (sPlanet*)(const char*)sPlanets.planets;
  // draw Planet Texture
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, info.textures[TEX_PLANET].id);
  glBegin(GL_QUADS);
    const int R = PLANET_RADIUS; // radius
    
    for(unsigned int i=0; i<sPlanets.size; i++){
      glColor3ub(100+planets[i].party*155,255-planets[i].party*155,100);
      glTexCoord2f(0.0,0.0); glVertex2i(dX+planets[i].x-R,dY+planets[i].y-R);
      glTexCoord2f(1.0,0.0); glVertex2i(dX+planets[i].x+R,dY+planets[i].y-R);
      glTexCoord2f(1.0,1.0); glVertex2i(dX+planets[i].x+R,dY+planets[i].y+R);
      glTexCoord2f(0.0,1.0); glVertex2i(dX+planets[i].x-R,dY+planets[i].y+R);
    }
  glEnd();
  glDisable(GL_TEXTURE_2D);
  // draw line to gathering point for created ships
  glBegin(GL_LINES);
    glColor3ub(100,100,100);
    for(unsigned int i=0; i<sPlanets.size; i++){
      glVertex2i(dX+planets[i].x ,dY+planets[i].y );
      glVertex2i(dX+planets[i].tx,dY+planets[i].ty);
    }
  glEnd();
  // draw Health and other numbers onto planet
  for(unsigned int i=0; i<sPlanets.size; i++){
    glColor3ub(250,250,25);
    drawInt(planets[i].party,     dX+planets[i].x,      dY+planets[i].y-R, 1);
    drawInt(planets[i].level[0],  dX+planets[i].x-R+20, dY+planets[i].y-R/2, 1);
    drawInt(planets[i].level[1],  dX+planets[i].x-R+30, dY+planets[i].y-R/2, 1);
    drawInt(planets[i].level[2],  dX+planets[i].x-R+40, dY+planets[i].y-R/2, 1);
    glColor3ub(0,200,0);                          
    drawInt(planets[i].health,    dX+planets[i].x-R+10, dY+planets[i].y, 1);
    glColor3ub(0,0,200);    
    drawInt(planets[i].power,     dX+planets[i].x,      dY+planets[i].y, 1);
    glColor3ub(100,0,200);
    drawString(planets[i].shieldActive?"(O)":" O ",3,   dX+planets[i].x, dY+planets[i].y+R/2, 1);
    glColor3ub(200,255,200);    
    drawInt(planets[i].shipQueue, dX+planets[i].x-R/2,  dY+planets[i].y+R/2, 1);
  }
}
void drawShips(const saShip * sShips, const float dX, const float dY){
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, info.textures[TEX_SHIP].id);
  glBegin(GL_QUADS);
    for(unsigned int party=0; party<PN; party++) {
      sShip * ships = sShips[party].ships;
      for(unsigned int i=0; i<sShips[party].size; i++){
        if(ships[i].health>0){
          glColor3ub(party * 255, (1 - party) * 255, (10 - ships[i].health) * 25);
          glTexCoord2f(0.0,0.0); glVertex2i(dX+ships[i].x-SHIP_RADIUS,dY+ships[i].y-SHIP_RADIUS);
          glTexCoord2f(1.0,0.0); glVertex2i(dX+ships[i].x+SHIP_RADIUS,dY+ships[i].y-SHIP_RADIUS);
          glTexCoord2f(1.0,1.0); glVertex2i(dX+ships[i].x+SHIP_RADIUS,dY+ships[i].y+SHIP_RADIUS);
          glTexCoord2f(0.0,1.0); glVertex2i(dX+ships[i].x-SHIP_RADIUS,dY+ships[i].y+SHIP_RADIUS);
        }
      }
    }
  glEnd();
  glDisable(GL_TEXTURE_2D); 
}
void drawShots(const saShot * sShots, const float dX, const float dY){
  glDisable(GL_BLEND); // shots are not blended at all
  glBegin(GL_LINES);
    for(unsigned int party=0; party<PN; party++) {
      glColor3ub(party*255,255-party*255,0);
      sShot * shots = sShots[party].shots;
      for(unsigned int i=0; i<sShots[party].size; i++){
        if(shots[i].timeToLive>0) {
          glVertex2i(dX+shots[i].x,dY+shots[i].y);
          glVertex2i(dX+shots[i].x+shots[i].dx*SHOT_LENGTH/SHOT_SPEED,dY+shots[i].y+shots[i].dy*SHOT_LENGTH/SHOT_SPEED);
          //drawInt(i, shots[i].x, shots[i].y); // draw ID instead, glEnd needed!
        }
      }
    }
  glEnd();
  glEnable(GL_BLEND);
}

void loadTextures(){
  for(int i=0;i<TEX_AMOUNT;i++)
    info.textures[i]=loadTexture(textureNames[i]);
}
struct sTexture loadTexture(const char * file){
  struct sTexture tex;
  unsigned char *data = stbi_load(file, &tex.w, &tex.h, &tex.n, 0);
  glGenTextures(1,&tex.id);
  glBindTexture(GL_TEXTURE_2D, tex.id);
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
  stbi_image_free(data);
  return tex;
}

struct sInfo * getInfo(){
  return &info;
}
// Exits program with status and printed error message
void exit(const char * msg, int status){
  printf(msg);
  exit(status);
}
// GLFWs Error-Callback, just printing the error
void cb_error(int error, const char * description){
  printf("Error(%i):\"%s\"\n", error, description);
}

void drawInt(int i, float strX, float strY, float stretchXY){
  drawInt(i, strX, strY, stretchXY, stretchXY);
}
void drawInt(int i, float strX, float strY, float stretchX, float stretchY){
    char s[12];
    sprintf(s,"%i",i);
    drawString(s,strlen(s),strX,strY,stretchX, stretchY);
}
void drawString(const char* str, unsigned int strlen, float strX, float strY, float stretchXY){
    drawString(str,strlen,strX,strY,stretchXY,stretchXY);
}
void drawString(const char* str, unsigned int strlen, float strX, float strY, float stretchX, float stretchY){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, info.textures[TEX_FONT].id);               // Select Our Texture
    glBegin(GL_QUADS);
        for(unsigned int i=0;i<strlen;i++){
            char c = str[i];
            float cx = ((c-' ')%16)/16.0f; // 16 chars per row, relative to width=1.0f
            float cy = ((c-' ')/16)/6.0f; // 16 chars per row, 6 per col, relative to height=1.0f
            float dx = 8/128.0f; // width one char:   8px of 128px per row
            float dy = 16/102.0f; // height one char: 16px of  102px per col
            float sx = strX+i*8*stretchX; //screenX - where to plot
            float sy = strY;     //screenY

            glTexCoord2f(cx, cy);
            glVertex2d(sx,sy); // ul

            glTexCoord2f(cx+dx, cy);
            glVertex2d(sx+8*stretchX, sy); // ur

            glTexCoord2f(cx+dx,cy+dy);
            glVertex2d(sx+8*stretchX, sy+16*stretchY); // or

            glTexCoord2f(cx, cy+dy);
            glVertex2d(sx, sy+16*stretchY); // ol
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
//////////////////////////////
//// random number generation
/////////////////////////////
float randf(){ // within -1 and 1
  return ((signed int)rand(-1000000000 ,1000000000))/(float)(2000000000);
}
int rand(int min, int max){
  return ((signed int)rand(max-min)+min);
}

unsigned int rand(unsigned int max){
  return xorshf96()%max;
}
unsigned long xorshf96(void) {          //period 2^96-1
// Marsaglia's xorshf generator
//http://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
  static unsigned long x=123456789, y=362436069, z=521288629;
  unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}

/*
  sPlanet * planets = new sPlanet[3];
  planets[0]= sPlanet{0,100,100,150,120,1,1,0,0,0,100,100,false};
  planets[1]= sPlanet{0,270,170,160,130,1,1,3,1,0,99,50,true};
  planets[2]= sPlanet{0,140,280,130,110,1,1,0,2,0,100,100,false};
  
  sShip * ships = new sShip[2];
  ships[0]={0,20,30,0,0,20,30,5};
  ships[1]={0,40,30,0,0,20,30,10};
  
  sShot * shots = new sShot[2];
  shots[0] = {2,10,10,5,5};
  shots[1] = {1,15,10,4,-3};

*/


