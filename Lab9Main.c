// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name
// Last Modified: 12/26/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "images/newimages.h"


// #include "../inc/JoyStick.c"



// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}
uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

// use main2 to observe graphics
int main(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  // JoyStick_Init();
  uint32_t joyx, joyy;
    //note: if you colors are weird, see different options for
  ST7735_InvertDisplay(1); // inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  
  // ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  // ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  // ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  // ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  // ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  // ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  // ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  // ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  // ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  // ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);


  // ST7735_DrawBitmap(0, 159, spacey, 128,160);
  int xwingx = 15;
  int xwingy = 133;
  
  int16_t shotcenter_x = 64;
  int16_t shotcenter_y = 70;

  int16_t tlcannonx = xwingx + 15;
  int16_t tlcannony = xwingy - 50;

  int16_t trcannonx = xwingx + 95;
  int16_t trcannony = xwingy - 50;

  int16_t blcannonx = xwingx + 15;
  int16_t blcannony = xwingy - 20; 

  int16_t brcannonx = xwingx + 80;
  int16_t brcannony = xwingy - 20; 

  ST7735_DrawBitmapTransparent(xwingx, xwingy, xwing, 100,53, 0x0000,0);

  int32_t x = -20;
  while(1){
    // SmallFont_OutVertical(t,104,6); // top left
  // Clock_Delay1ms(50);              // delay 50 msec
  // JoyStick_In(&joyx, &joyy);
  // printf("%d",joyx);
  
// Draw lines with color 0x07C0 (for example, drawing the shot)
  ST7735_Line(tlcannonx, tlcannony, shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/20), 0x0000);
  ST7735_Line(shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/20), 
              shotcenter_x + (double)(shotcenter_x - tlcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - tlcannony)*(double)((double)x/40), 0xBC1F);

  ST7735_Line(trcannonx, trcannony, shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)((double)x/20), 0x0000);
  ST7735_Line(shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)( (double)x/20), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)( (double)x/20), 
              shotcenter_x + (double)(shotcenter_x - trcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - trcannony)*(double)((double)x/40), 0xBC1F);



  ST7735_Line(blcannonx, blcannony, shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/20), 0x0000);
  ST7735_Line(shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/20), 
              shotcenter_x + (double)(shotcenter_x - blcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - blcannony)*(double)((double)x/40), 0x07C0);

  ST7735_Line(brcannonx, brcannony, shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/20), 0x0000);
  ST7735_Line(shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/20), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/20), 
              shotcenter_x + (double)(shotcenter_x - brcannonx)*(double)((double)x/40), shotcenter_y + (double)(shotcenter_y - brcannony)*(double)((double)x/40), 0x07C0);

  ST7735_DrawBitmapPartial(0, 0, blue_space, 128, 160, 0, 0, 128, 160);

// ST7735_Line(xwingx + 5 + ((shotcenter_x - xwingx) * ((double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 5),
//              shotcenter_x, shotcenter_y, 0x07C0);

// ST7735_Line(xwingx + 95 + ((shotcenter_x - xwingx) * ((double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 5),
//              shotcenter_x, shotcenter_y, 0x07C0);

// Draw the same lines with color 0x0000 (for erasing, for instance)
// ST7735_Line(xwingx + ((shotcenter_x - xwingx) * (1 - (double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 45),
//              shotcenter_x, shotcenter_y, 0x0000);

// ST7735_Line(xwingx + 100 + ((shotcenter_x - xwingx) * (1- (double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 45),
//              shotcenter_x, shotcenter_y, 0x0000);

// ST7735_Line(xwingx + 5 + ((shotcenter_x - xwingx) * (1 -(double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 5),
//              shotcenter_x, shotcenter_y, 0x0000);

// ST7735_Line(xwingx + 95 + ((shotcenter_x - xwingx) * (1 - (double)(x % 10) / 10)),
//              shotcenter_y - (xwingy - 5),
//              shotcenter_x, shotcenter_y, 0x0000);



  ST7735_DrawBitmapTransparent(xwingx + 29, xwingy -32,jet, 15,14, 0x0000, (double)(x%20)/50);
  ST7735_DrawBitmapTransparent(xwingx + 57, xwingy -32, jet, 15,14, 0x0000,(double)(x%20)/50 );
  ST7735_DrawBitmapTransparent(xwingx + 29, xwingy , jet, 15,14, 0x0000,(double)(x%20)/50);
  ST7735_DrawBitmapTransparent(xwingx + 57, xwingy , jet, 15,14, 0x0000,(double)(x%20)/50 );
  Clock_Delay1ms(10); 
  if (x != 0){
    x++;
  }else{
    x = -20;
  }
  }
  
  // ST7735_FillScreen(0x0000);   // set screen to black
  // ST7735_SetCursor(1, 1);
  // ST7735_OutString("GAME OVER");
  // ST7735_SetCursor(1, 2);
  // ST7735_OutString("Nice try,");
  // ST7735_SetCursor(1, 3);
  // ST7735_OutString("Earthling!");
  // ST7735_SetCursor(2, 4);
  // ST7735_OutUDec(1234);
  while(1){
  }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs
    
  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main5(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
