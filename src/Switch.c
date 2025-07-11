/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PB4INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PB1INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00040081;
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
  uint32_t result = 0;
  if (GPIOB->DIN31_0 & (1 << 4)){
    result |= 0x01; 
  }
  if (GPIOB->DIN31_0 & (1 << 1)){
    result |= 0x02;
  }
  if (GPIOA->DIN31_0 & (1 << (28))){
    result |= 0x04;
  }
  return result;
}
