#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/devices/msp/msp.h>
#include <math.h>
#include <string.h>       // memset
#include <stdbool.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/ST7735.h"
#include "UART1.h"
#include "UART2.h"
#include "Switch.h"
#include "JoyStick.h"
#include "assets/newimages2.h"    // spacey[], xwing_front[], FRONT_W, FRONT_H
#include "Sound.h"
#include "EdgeTriggered.h"


#define FRONT_W     128
#define FRONT_H      64
#define BACK_W     128
#define BACK_H      64
#define RIGHT_W     64
#define RIGHT_H      128
#define LEFT_W     64
#define LEFT_H      128
#define LCD_W       128
#define LCD_H       160
#define STAR_COUNT  200
#define WORLD_SIZE  500.0f
#define MAP_SIZE      5
#define HALF_WORLD  (WORLD_SIZE*0.5f)
#define CELL_SIZE   (WORLD_SIZE/MAP_SIZE)
#define BOOST_FACTOR 2.0f
#define REMOTE_THRESHOLD 1000
#define FRAME_MARKER 0xAB
#define MIN_Z       20.0f
#define MIN_SEP        75.0f      // minimum allowed centre-to-centre distance //
#define MIN_SEP2       (MIN_SEP*MIN_SEP)

static uint8_t map3D[MAP_SIZE][MAP_SIZE][MAP_SIZE];

void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

volatile uint8_t primaryflag = 0;
volatile uint8_t secondaryflag = 0;      
volatile uint8_t boostflag = 0;     

int fighter = 0;

////////////////////////////////
int aiFrameCounter = 0;
const int AI_UPDATE_INTERVAL = 5;
////////////////////////////////

void GROUP1_IRQHandler(void){
  if(GPIOB->CPU_INT.RIS & (1<<4)){
    primaryflag = 1;
    GPIOB->CPU_INT.ICLR = (1<<4); 
  } 
  if(GPIOB->CPU_INT.RIS & (1<<1)){
    secondaryflag = 1;
    GPIOB->CPU_INT.ICLR = (1<<1);
  } 
  if(GPIOA->CPU_INT.RIS & (1<<28)){
    boostflag = 1;
    GPIOA->CPU_INT.ICLR = (1<<28);
  } 
}

#define SHOT_STEPS_PER_FRAME 8

//-------------------------health bar-----------------------------------
#define MAX_HEALTH 100
#define HEALTH_BAR_X_START 24
#define HEALTH_BAR_Y 8
#define HEALTH_BAR_WIDTH 90

uint8_t PlayerHealth = 100;

//cannon/xwing coords--------------------------------------------------
int16_t shotcenter_x = 64;
  int16_t shotcenter_y = 80;

  int16_t tlcannonx = 0;
  int16_t tlcannony = 40;

  int16_t trcannonx = 128;
  int16_t trcannony = 40;

  int16_t blcannonx = 0;
  int16_t blcannony = 120; 

  int16_t brcannonx = 128;
  int16_t brcannony = 120; 

  int x = -20; 
  //-------------------------------------------------------------------


// quantize relative coordinate
static inline int toCell(float rel){
    int c = (int)((rel + HALF_WORLD) / CELL_SIZE);
    if (c < 0)           c = 0;
    else if (c >= MAP_SIZE) c = MAP_SIZE-1;
    return c;
}

typedef struct { float x, y, z; } Vec3;
static Vec3 stars[STAR_COUNT];

typedef struct { 
    float x, y, z, yaw; 
    uint8_t health;
} Ship_t;
static Ship_t opponent = { 0, 0, 250.0f, 0, 100};

typedef struct {
    float x, y, z;
    float yaw;
    float pitch;
    uint8_t health;
} Cam_t;
static Cam_t cam = { 0, 0, -250.0f, 0, 0, 100};

const float YAW_STEP   = 0.05f;
const float PITCH_STEP = 0.05f;
const float MOVE_SPEED = 3.0f;
const float FOCAL      = 100.0f;

bool shotStarted = false;
bool hitRegistered = false;

//Health bar stuff

void HealthBar_Init(void){
  PlayerHealth = MAX_HEALTH;
  ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y,
              HEALTH_BAR_X_START + HEALTH_BAR_WIDTH, HEALTH_BAR_Y,
              ST7735_RED);
  ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y+1,
              HEALTH_BAR_X_START + HEALTH_BAR_WIDTH, HEALTH_BAR_Y+1,
              ST7735_RED);
  ST7735_DrawBitmapTransparent(8, 14, heart, 12, 12, ST7735_BLACK, 0);
}

void HealthBar_Update(uint8_t health){
  if (health > MAX_HEALTH) health = MAX_HEALTH;

  ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y,
              HEALTH_BAR_X_START + HEALTH_BAR_WIDTH, HEALTH_BAR_Y,
              ST7735_BLACK);
  ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y+1,
              HEALTH_BAR_X_START + HEALTH_BAR_WIDTH, HEALTH_BAR_Y+1,
              ST7735_BLACK);

  uint8_t length = (health * HEALTH_BAR_WIDTH) / MAX_HEALTH;
  if (length > 0) {
    ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y,
                HEALTH_BAR_X_START + length, HEALTH_BAR_Y,
                ST7735_RED);
    ST7735_Line(HEALTH_BAR_X_START, HEALTH_BAR_Y+1,
                HEALTH_BAR_X_START + length, HEALTH_BAR_Y+1,
                ST7735_RED);

  }
}
//---------------------------------------------------------------------
int x0, x1, y0, y1 = 0;

uint16_t *sprite;
int sprW = 0, sprH = 0;

static void DrawOpponent(void){
    float wx = opponent.x - cam.x;
    float wy = opponent.y - cam.y;
    float wz = opponent.z - cam.z;

    //yaw
    float cx =  wx * cosf(cam.yaw) + wz * sinf(cam.yaw);
    float cz = -wx * sinf(cam.yaw) + wz * cosf(cam.yaw);

    //pitch
    float cy =  wy * cosf(cam.pitch) - cz * sinf(cam.pitch);
    float cz2 = wy * sinf(cam.pitch) + cz * cosf(cam.pitch);

    if (cz2 <= 0.0f)    return;         
    if (cz2 <  MIN_Z)   cz2 = MIN_Z;    

    const uint16_t *sprite = xwing_front;
    int sprW = FRONT_W, sprH = FRONT_H;

    float vx = -cx;                
    float vz = -cz2;            
    float invI = 1.0f / sqrtf(vx*vx + vz*vz);
    vx *= invI;  vz *= invI;

    float fx =  sinf(opponent.yaw);
    float fz =  cosf(opponent.yaw);

    float dot   = fx*vx + fz*vz;          
    float cross = fx*vz - fz*vx;       


    float scale = FOCAL / cz2;
    float invS  = cz2 / FOCAL;          

    int w = (int)(sprW * scale); if (w < 1) w = 1;
    int h = (int)(sprH * scale); if (h < 1) h = 1;

    int scrX = (int)( cx * scale + LCD_W/2 - w/2);
    int scrY = (int)( cy * scale + LCD_H/2 - h/2);

    x0 = scrX < 0       ? 0       : scrX;
    x1 = scrX + w > LCD_W ? LCD_W : scrX + w;
    y0 = scrY < 0       ? 0       : scrY;
    y1 = scrY + h > LCD_H ? LCD_H : scrY + h;
    if (x1 <= x0 || y1 <= y0) return;

    for (int py = y0; py < y1; ++py){
        int sy = (int)(((py - scrY) + 0.5f) * invS);
        if (sy >= sprH) sy = sprH - 1;
        if( py < 16 || py >= LCD_H) continue;
        for (int px = x0; px < x1; ++px){
            int sx = (int)(((px - scrX) + 0.5f) * invS);
            if (sx >= sprW) sx = sprW - 1;

            uint16_t c = sprite[sy * sprW + sx];   
            if (c) ST7735_DrawPixel(px, py, c);
        }
    }
}

float UART_Get(void) {
    uint8_t nibbles[8] = {0};

    for (int i = 0; i < 8; i++) {
        uint8_t b   = UART2_InChar();
        uint8_t idx = (b >> 4) & 0x07;      // force 0–7
        uint8_t val =  b       & 0x0F;
        nibbles[idx] = val;
    }

    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = nibbles[2*i] | (nibbles[2*i+1] << 4);
    }

    float result;
    memcpy(&result, bytes, sizeof result);
    return result;
}

float UART_Post(float f){
    
    uint8_t* bytePtr = (uint8_t*)&f;

    for(int i = 0; i < 8; i++){
        UART1_OutChar((i << 4) | (i % 2 ? (bytePtr[i/2] >> 4) : (bytePtr[i/2] & 0x0F)));
        Clock_Delay1ms(1);
    } 
}

void GameOver(uint8_t playerHealth) {
    
  const char* lines[4];
  uint16_t color = ST7735_WHITE;
  
  if (playerHealth <= 0) {
    lines[0] = "MISSION FAILED";
    lines[1] = "The Force was not";
    lines[2] = "with you...";
    lines[3] = NULL;
    color = ST7735_RED;
  } else {
    lines[0] = "VICTORY!";
    lines[1] = "The force is strong";
    lines[2] = "with this one...";
    lines[3] = NULL;
    color = ST7735_GREEN;
  }

  int startRow = 18; 
  int staticRow = 3;  

  // Scrolling logic
  for (int scroll = 0; scroll <= startRow; scroll++) {
    ST7735_FillScreen(ST7735_BLACK);

    for (int i = 0; lines[i] != NULL; i++) {
      int y = (startRow - scroll) + i * 2;

      //Stop the scrolling when it reaches the static row
      if (y >= staticRow) {
        ST7735_DrawString(1, y, (char*)lines[i], color);
      }
    }

   
    if (startRow - scroll <= staticRow) {
      break; 
    }

    Clock_Delay1ms(160); //Scroll speed
  }

  ST7735_DrawString(3, 13, "Press any key", ST7735_YELLOW);
  ST7735_DrawString(5, 14, "to return", ST7735_YELLOW);

  while(Switch_In() == 0) {
    Clock_Delay1ms(50);
  }
  while(Switch_In() != 0) {
    Clock_Delay1ms(50);
  }
  ST7735_FillScreen(ST7735_BLACK);
  return;
}

void SpriteCheck(int8_t choose){
    if(choose){
        sprite = tiefront;
        sprW = 64;
        sprH = 64;
    }else{
        sprite = xwing_front;
        sprW = FRONT_W;
        sprH = FRONT_H;
    }
}

////////////////////////////////////////////////////
void AI_FireAtPlayer(void) {
    float dx = cam.x - opponent.x;
    float dy = cam.y - opponent.y;
    float dz = cam.z - opponent.z;

    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    if (dist < 250.0f) { // only fire if within range
        float aimThreshold = 0.95f;

        // Vector from AI to player (normalized)
        float vx = dx / dist;
        float vz = dz / dist;

        // AI's forward vector
        float fx = sinf(opponent.yaw);
        float fz = cosf(opponent.yaw);

        float dot = fx * vx + fz * vz;

        if (dot > aimThreshold) {
            // Deal damage
            if (cam.health > 0) {
                cam.health -= 10;
                HealthBar_Update(cam.health);

                if (cam.health == 0) {
                    Sound_explode();
                    GameOver(cam.health);
                    mainmenu();
                }
            }
        }
    }
}
//////////////////////////////////////////
//////////////////////////////////////////
void AI_Update(void) {
    // Vector from AI to player
    float dx = cam.x - opponent.x;
    float dy = cam.y - opponent.y;
    float dz = cam.z - opponent.z;

    float d2 = dx*dx + dy*dy + dz*dz;

    // Only move if far enough away
    if (d2 > MIN_SEP2 * 1.2f) {
        float dist = sqrtf(d2);
        float moveSpeed = 1.0f;  // slower than player

        opponent.x += (dx / dist) * moveSpeed;
        opponent.y += (dy / dist) * moveSpeed;
        opponent.z += (dz / dist) * moveSpeed;
    }

    // Aim at the player
    opponent.yaw = atan2f(cam.x - opponent.x, cam.z - opponent.z);

    // Occasionally fire a shot at the player
    static int shotCooldown = 0;
    if (--shotCooldown <= 0) {
        AI_FireAtPlayer();
        shotCooldown = 50; // Wait 50 frames between shots
    }
}
////////////////////////////////////////////////////

int maingame(int fighter) {
    // hardware init
    Sound_Init();
    HealthBar_Init();
    EdgeTriggered_Init();
    SpriteCheck(0);

    // init starfield
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = (rand()/(float)RAND_MAX - 0.5f) * WORLD_SIZE;
        stars[i].y = (rand()/(float)RAND_MAX - 0.5f) * WORLD_SIZE;
        stars[i].z = 50.0f + (rand()/(float)RAND_MAX) * 400.0f;
    }

    // initialize remote logs to our start pos
    int16_t rx_x = (int16_t)cam.x;
    int16_t rx_y = (int16_t)cam.y;
    int16_t rx_z = (int16_t)cam.z;

    while (1) {
        ////////////////////////////////////////////
        if (++aiFrameCounter >= AI_UPDATE_INTERVAL) {
          aiFrameCounter = 0;
          AI_Update();
        }
        /////////////////////////////////////////////
        int joy = readjoystick();
        if      (joy == 8)  cam.pitch -= PITCH_STEP;
        else if (joy == 2)  cam.pitch += PITCH_STEP;
        else if (joy == 4)  cam.yaw += YAW_STEP;
        else if (joy == 6)  cam.yaw -= YAW_STEP;

  
        if (cam.pitch >  1.30f) cam.pitch =  1.30f;
        if (cam.pitch < -1.30f) cam.pitch = -1.30f;

        boostflag = (Switch_In() & 0x04);
        if (boostflag){
        float step = MOVE_SPEED * BOOST_FACTOR;

        float dx =  step *  sinf(cam.yaw) * cosf(cam.pitch);  // east-west
        float dy =  step * -sinf(cam.pitch);                  // up-down (negative = up)
        float dz =  step *  cosf(cam.yaw) * cosf(cam.pitch);  // north-south


        float candX = cam.x + dx;
        float candY = cam.y + dy;
        float candZ = cam.z + dz;


        float d2 = (candX - opponent.x)*(candX - opponent.x)
                    + (candY - opponent.y)*(candY - opponent.y)
                    + (candZ - opponent.z)*(candZ - opponent.z);

        if (d2 >= MIN_SEP2){
            cam.x = candX;
            cam.y = candY;
            cam.z = candZ;
        }

    }

        // clamp camera inside world
        cam.x = fmaxf(fminf(cam.x, HALF_WORLD), -HALF_WORLD);
        cam.y = fmaxf(fminf(cam.y, HALF_WORLD), -HALF_WORLD);
        cam.z = fmaxf(fminf(cam.z, HALF_WORLD), -HALF_WORLD);

    
    // UART_Post(cam.x);
    // UART_Post(cam.y);
    // UART_Post(cam.z);
    // opponent.x = UART_Get();
    // opponent.y = UART_Get();
    // opponent.z = UART_Get();


{
    static uint8_t prevSw = 0;
    uint8_t curSw  = Switch_In() & 0x02;     // sw2 pressed?
    if (curSw && !prevSw) {                  // rising edge only
        const float RECENTER_DIST = 75.0f;   // desired separation

        float dx = opponent.x - cam.x;
        float dy = opponent.y - cam.y;
        float dz = opponent.z - cam.z;

        cam.yaw   = atan2f(dx, dz);                          // +Y axis up
        float horiz = sqrtf(dx*dx + dz*dz);                  // projection in XZ
        cam.pitch = atan2f(-dy, horiz);                      // positive = look up

        float fx =  sinf(cam.yaw) * cosf(cam.pitch);
        float fy = -sinf(cam.pitch);
        float fz =  cosf(cam.yaw) * cosf(cam.pitch);

        // cam.x = opponent.x - fx * RECENTER_DIST;
        // cam.y = opponent.y - fy * RECENTER_DIST;
        // cam.z = opponent.z - fz * RECENTER_DIST;

        // cam.x = fmaxf(fminf(cam.x, HALF_WORLD), -HALF_WORLD);
        // cam.y = fmaxf(fminf(cam.y, HALF_WORLD), -HALF_WORLD);
        // cam.z = fmaxf(fminf(cam.z, HALF_WORLD), -HALF_WORLD);

        float d2 = (cam.x - opponent.x)*(cam.x - opponent.x)
                + (cam.y - opponent.y)*(cam.y - opponent.y)
                + (cam.z - opponent.z)*(cam.z - opponent.z);

        if (d2 < MIN_SEP2){
            float gap = MIN_SEP - sqrtf(d2);
            cam.x -= fx * gap;
            cam.y -= fy * gap;
            cam.z -= fz * gap;
        }

    }
    prevSw = curSw;
}

    // {
    // static uint8_t prevSw = 0;
    // uint8_t curSw = Switch_In() & 0x02;   
    // if (curSw && !prevSw) {               //rising edge only
    //     cam.yaw   = 0.0f;                 //face +Z
    //     cam.pitch = 0.0f;                 //level
    // }
    // prevSw = curSw;
    // }


       ST7735_FillRect(0, 16, LCD_W, LCD_H - 16, ST7735_BLACK);
       ST7735_DrawBitmapTransparent(59, 85, crosshair, 11, 11, ST7735_WHITE, 0);
        memset(map3D, 0, sizeof(map3D));
   
        for(int i=0;i<STAR_COUNT;i++){
            float wx = stars[i].x - cam.x;
            float wy = stars[i].y - cam.y;
            float wz = stars[i].z - cam.z;

            float yyaw = -cam.yaw;
            float rotZ = wx*sinf(yyaw) + wz*cosf(yyaw);
            float rotX = wx*cosf(yyaw) - wz*sinf(yyaw);

            float xpitch = -cam.pitch;
            float rotY = wy*cosf(xpitch) - rotZ*sinf(xpitch);
                  rotZ = wy*sinf(xpitch) + rotZ*cosf(xpitch);

            if(rotZ < MIN_Z) continue;                 // behind near-plane
            float inv = FOCAL / (fabsf(rotZ) + FOCAL);
            int sx = (int)(rotX * inv + LCD_W/2);
            int sy = (int)(rotY * inv + LCD_H/2);

            if(sx>=0 && sx<LCD_W && sy>=16 && sy<LCD_H)
                ST7735_DrawPixel(sx, sy, ST7735_WHITE);
        }

        DrawOpponent();
        ST7735_DrawBitmapTransparent(59, 85, crosshair, 11, 11, ST7735_WHITE, 0);

    //blasts
    for(int i = 0; i<SHOT_STEPS_PER_FRAME; i++){
        if(primaryflag && (x < 0)){
            if(!shotStarted){
                if(fighter == 1){
                    Sound_tiefightershoot();
                shotStarted = true;
                hitRegistered = false;
                } else if(fighter == 0){
                    Sound_xwingshoot();
                    shotStarted = true;
                    hitRegistered = false;
                }
            }
            ST7735_Line(tlcannonx, tlcannony, shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/20), 0x0000);
            ST7735_Line(shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/20), 
                shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/40), 0xBC1F);

            ST7735_Line(trcannonx, trcannony, shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)((double)x/20), 0x0000);
            ST7735_Line(shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)( (double)x/20), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)( (double)x/20), 
                shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)((double)x/40), 0xBC1F);

            ST7735_Line(blcannonx, blcannony, shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/20), 0x0000);
            ST7735_Line(shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/20), 
                shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/40), 0xBC1F);

            ST7735_Line(brcannonx, brcannony, shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/20), 0x0000);
            ST7735_Line(shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/20), 
                shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/40), 0xBC1F);

            if(!hitRegistered && shotcenter_x > x0 && shotcenter_x < x1 && shotcenter_y > y0 && shotcenter_y < y1){
                opponent.health -= 10;
                HealthBar_Update(opponent.health);
                if(opponent.health == 0 || cam.health == 0){
                    Sound_explode();
                    GameOver(cam.health);
                    mainmenu();
                }
                hitRegistered = true;
            }
            x++;
        } else {
            x = -20;
            shotStarted = false;
            primaryflag = 0;
        } 
    }

    //draw crosshair
    ST7735_DrawBitmapTransparent(59, 85, crosshair, 11, 11, ST7735_WHITE, 0);
    }
    return 0;
}

const char* english[3] = {"Play", "Controls", "Espa\xA4ol"};
const char* spanish[3] = {"Jugar", "Controles", "English"};

void DrawMenu(int selected, int prevselected, int language) {
    const char** menu = (language == 0) ? english : spanish;

    if (prevselected >= 0 && prevselected < 3) {
        ST7735_DrawString(5, 8 + prevselected * 2, " ", ST7735_BLACK); 
        ST7735_DrawString(7, 8 + prevselected * 2, (char*)menu[prevselected], ST7735_WHITE);
    }

    ST7735_DrawString(5, 8 + selected * 2, ">", ST7735_YELLOW);
    ST7735_DrawString(7, 8 + selected * 2, (char*)menu[selected], ST7735_YELLOW);
}

int Fighter_Select_Screen(void){
int selected = 0;   
ST7735_FillScreen(ST7735_BLACK);
ST7735_DrawBitmap(0, 70, xwing_front, 128, 64); 
ST7735_DrawBitmap(34, 145, tiefront, 64, 64); 
ST7735_DrawString(6, 6, "X-Wing", ST7735_WHITE);
ST7735_DrawString(6, 15, "TIE Fighter", ST7735_WHITE);
while(1){
  int input = readjoystick();
  if(input == 2 || input == 8){
    selected ^= 1;
  }
    ST7735_DrawString(4, 6, " ", ST7735_BLACK);
    ST7735_DrawString(4, 15, " ", ST7735_BLACK);
    ST7735_DrawString(6, 6, "X-Wing", ST7735_WHITE);
    ST7735_DrawString(6, 15, "TIE Fighter", ST7735_WHITE);
  if(selected == 0){
    ST7735_DrawString(4, 6, ">", ST7735_YELLOW);
    ST7735_DrawString(6, 6, "X-Wing", ST7735_YELLOW);
  }else {
    ST7735_DrawString(4, 15, ">", ST7735_YELLOW);
    ST7735_DrawString(6, 15, "TIE Fighter", ST7735_YELLOW);
  }

  int button = Switch_In();
  if(button & 0x01){
    return selected;
  }

  Clock_Delay1ms(200);
  }
}

int mainmenu(void){
  int selected = 0;
  int language = 0; //initialize to english
  int prevselected = 0;

  ST7735_DrawBitmap(0, 80, Title, 128, 70); //title
  for (int i = 0; i < 3; i++) { //buttons
    ST7735_DrawString(7, 8 + i * 2, (char*)((language == 0) ? english[i] : spanish[i]), ST7735_WHITE);
  }
  DrawMenu(selected, -1, language);
while(1){
int input = readjoystick();
    if (input == 1 || input == 2 || input == 3) { // up
      selected = (selected + 3 - 1) % 3;
    } else if (input == 7 || input == 8 || input == 9) { // down
      selected = (selected + 1) % 3;
    } 

    //if switching languages
    int button = Switch_In();
    if (button & 0x01 && selected == 2){
        language ^= 1; //change languages

        ST7735_FillScreen(ST7735_BLACK);
        if(language == 0){
          ST7735_DrawBitmap(0, 80, Title, 128, 70);
        } else if(language == 1){
          //ST7735_DrawBitmap(0, 80, spanishTitle, 128, 70);
          ST7735_DrawBitmap(0, 80, Title, 128, 70);
        }
        for (int i = 0; i < 3; i++) {
          ST7735_DrawString(7, 8 + i * 2, (char*)((language == 0) ? english[i] : spanish[i]), ST7735_WHITE);
        }
        DrawMenu(selected, -1, language);
        prevselected = selected;
    }

    if(button & 0x01 && selected == 1) {
    ST7735_FillScreen(ST7735_BLACK);  // clear screen

    ST7735_DrawString(4, 1, "== CONTROLS ==", ST7735_YELLOW);

    ST7735_DrawString(2, 3, "Mission: destroy", ST7735_WHITE);
    ST7735_DrawString(2, 4, "the enemy ship!", ST7735_WHITE);
    ST7735_DrawString(1, 6, "Joystick ", ST7735_CYAN);
    ST7735_DrawString(10, 6, "- Aim", ST7735_WHITE);
    ST7735_DrawString(1, 7, "L Button ", ST7735_CYAN);
    ST7735_DrawString(10, 7, "- Shoot", ST7735_WHITE);
    ST7735_DrawString(1, 8, "M Button ", ST7735_CYAN);
    ST7735_DrawString(10, 8, "- Re-Center", ST7735_WHITE);
    ST7735_DrawString(1, 9, "R Button ", ST7735_CYAN);
    ST7735_DrawString(10, 9, "- Movement", ST7735_WHITE);

    ST7735_DrawString(4, 11, "Press any key", ST7735_YELLOW);
    ST7735_DrawString(6, 12, "to return", ST7735_YELLOW);
    while(Switch_In() == 0) {
        Clock_Delay1ms(50);
    }
    while(Switch_In() != 0) {
        Clock_Delay1ms(50);
    }
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_DrawBitmap(0, 80, Title, 128, 70); //title
    for (int i = 0; i < 3; i++) { //buttons
    ST7735_DrawString(7, 8 + i * 2, (char*)((language == 0) ? english[i] : spanish[i]), ST7735_WHITE);
    }
    DrawMenu(selected, -1, language);
  }

    if(button & 0x01 && selected == 0){
    ST7735_FillScreen(ST7735_BLACK);
    fighter = Fighter_Select_Screen();  //0 = xwing, 1 = tie

    ST7735_FillScreen(ST7735_BLACK);
    ST7735_DrawString(2, 7, "Waiting for other", ST7735_WHITE);
    ST7735_DrawString(2, 9, "player...", ST7735_WHITE);

    // //wait for other player
    // bool receivedReady = false;
    // while (!receivedReady) {
    //     char incoming = UART2_InChar();
    //     if (incoming == 'R') {
    //         receivedReady = true;
    //         for(int i = 0; i<10; i++){
    //         UART1_OutChar('A'); 
    //         Clock_Delay1ms(11);
    //         }
    //     }
    //     Clock_Delay1ms(10);
    // }

    ST7735_FillScreen(ST7735_BLACK);
    maingame(fighter);
    }

    if(selected != prevselected){
    DrawMenu(selected, prevselected, language);
    prevselected = selected;
    }
    Clock_Delay1ms(200);
    }
}

//start menu
int main(void){
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  JoyStick_Init();
  Switch_Init();
  UART1_Init();
  UART2_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(ST7735_BLACK);
  __enable_irq();
  

  while(1)  {
    mainmenu();
  }
}

