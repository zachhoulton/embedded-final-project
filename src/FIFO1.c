// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: December 23, 2024
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>


// Declare state variables for FiFo
//        size, buffer, put and get indexes

#define FIFO_SIZE 32
static uint8_t PutI;  // index to put new
static uint8_t GetI;  // index of oldest
static char Fifo[FIFO_SIZE];

void Fifo1_Init(){
  PutI = GetI = 0;  // empty
}

uint8_t Fifo1_Put(char data){
  if(((PutI+1)%FIFO_SIZE) == GetI) return 0; // fail if full
  Fifo[PutI] = data;         // save in Fifo
  PutI = (PutI+1)%FIFO_SIZE; // next place to put
  return 1;
}

void Fifo1_Get(char *datapt){
  if(GetI == PutI) return ; // fail if empty
  *datapt = Fifo[GetI];      // retrieve data
  GetI = (GetI+1)%FIFO_SIZE; // next place to get
}

uint32_t Fifo1_Size(void){
  return (PutI-GetI)&(FIFO_SIZE-1);
}
