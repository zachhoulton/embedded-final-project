/* JoyStick.c
 * MKII BoosterPack
 * Jonathan Valvano
 * November 21, 2022
 * Derived from gpio_toggle_output_LP_MSPM0G3507_nortos_ticlang
 *              adc12_single_conversion_vref_internal_LP_MSPM0G3507_nortos_ticlang
 *              adc12_single_conversion_LP_MSPM0G3507_nortos_ticlang
 */



#include <ti/devices/msp/msp.h>
#include "../inc/JoyStick.h"
#include "../inc/Clock.h"
#include "../inc/ADC.h"
// Analog MKII  Joystick
// J1.5 joystick Select button (digital) PA26
// J1.2 joystick horizontal (X) (analog) PA25_ADC0.2
// J3.26 joystick vertical (Y) (analog)  PA18_ADC1.3
// remove J1 J2 J3 on LaunchPad to use PA26

// user buttons
// J4.33 user Button1 (bottom) (digital) PA12
// J4.32 user Button2 (top) (digital)    PA11 (conflicts with UART0 Rx from XDS Tx)

#define PA26INDEX 58
#define PA11INDEX 21
#define PA12INDEX 33
// Initialize MKII JoyStick and JoyStick button
void JoyStick_Init(void){
  ADC_Init(ADC0,2,ADCVREF_VDDA); // x position joystick
  ADC_Init(ADC1,3,ADCVREF_VDDA); // y position joystick
  // assume these are called from LaunchPad_Init
//  GPIOA->GPRCM.RSTCTL = (uint32_t)0xB1000003;  // Reset GPIOA
//  GPIOA->GPRCM.PWREN = (uint32_t)0x26000001;   // Enable power to GPIOA
  Clock_Delay(24); // time for gpio to power up

    // PINCM
    //   bit 25 is HiZ
    //   bit 20 is drive strength
    //   bit 18 is input enable control
    //   bit 17 is pull up control
    //   bit 16 is pull down control
    //   bit 7 is PC peripheral connected, enable transparent data flow
    //   bit 0 selects GPIO function
  IOMUX->SECCFG.PINCM[PA26INDEX]  = (uint32_t) 0x00040081;
  IOMUX->SECCFG.PINCM[PA11INDEX]  = (uint32_t) 0x00040081;
  IOMUX->SECCFG.PINCM[PA12INDEX]  = (uint32_t) 0x00040081;
}
#define JOYBUTTON (1<<26)
// Read JoyStick button
// Input: none
// Output: 0 if pressed, nonzero if not pressed
uint32_t JoyStick_InButton(void){
  return GPIOA->DIN31_0 & JOYBUTTON;
}

// Read JoyStick position
// Inputs: *x pointer to empty 32 bit unsigned variable
//         *y pointer to empty 32 bit unsigned variable
// Output: none
void JoyStick_In(uint32_t *x, uint32_t *y){
  *x = ADC_In(ADC0);
  *y = ADC_In(ADC1);
}
#define BUTT1 (1<<12)
// Read Button1 button, PA12
// Input: none
// Output: 0 if pressed, nonzero if not pressed
uint32_t JoyStick_InButton1(void){
  return GPIOA->DIN31_0 & BUTT1;
}
//#define BUTT2 (1<<11)
// Read Button2 button, PA11
// Input: none
// Output: 0 if pressed, nonzero if not pressed
//uint32_t JoyStick_InButton2(void){
//  return GPIOA->DIN31_0 & BUTT2;
//}

// Return values (same meaning as before)
//   1 2 3
//   4 5 6
//   7 8 9

int readjoystick(void){
  uint32_t xRaw, yRaw;
  JoyStick_In(&xRaw, &yRaw);            // one ADC sample each

  // --------------------------------------------------------------
  // Fine‑tune these three numbers for your board, if needed.
  // --------------------------------------------------------------
  const int32_t X_CENTER   = 2992;      // joystick X mid‑point
  const int32_t Y_CENTER   = 2663;      // joystick Y mid‑point (often ~200 ADC below X)
  const int32_t DEADBAND   =  350;      // ± range considered “neutral”
  // --------------------------------------------------------------

  int32_t dx = (int32_t)xRaw - X_CENTER;     // + → right,  – → left
  int32_t dy = (int32_t)yRaw - Y_CENTER;     // + → down,   – → up

  // Diagonals first: both axes outside dead‑zone
  if (dx >  DEADBAND && dy < -DEADBAND) return 3;   // up‑right
  if (dx < -DEADBAND && dy < -DEADBAND) return 1;   // up‑left
  if (dx >  DEADBAND && dy >  DEADBAND) return 9;   // down‑right
  if (dx < -DEADBAND && dy >  DEADBAND) return 7;   // down‑left

  // Straight directions
  if (dy < -DEADBAND) return 2;                     // up
  if (dy >  DEADBAND) return 8;                     // down
  if (dx >  DEADBAND) return 6;                     // right
  if (dx < -DEADBAND) return 4;                     // left

  // Within the neutral box → return 5
  return 5;
}


/*  updateAdditives.c
 *
 *  Call once each frame / loop
 *  Depends on: readjoystick()  (returns 1‑9 as described in JoyStick.c)
 */

void UpdateAdditiveXY(int *additiveX, int *additiveY){
    static const int8_t dx[10] = {0, -1, 0,  1, -1, 0,  1, -1, 0,  1};
    static const int8_t dy[10] = {0, -1, -1, -1,  0, 0,  0,  1, 1,  1};

    int joy = readjoystick();          // 1‒9 (center == 5)

    *additiveX = dx[joy];               // look‑up table → X delta
    *additiveY = dy[joy];               // look‑up table → Y delta
}