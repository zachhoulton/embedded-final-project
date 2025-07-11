/*!
 * @defgroup FIFO
 * @brief Lab 8 First in first out queue
 * @{*/
/**
 * @file      FIFO1.h
 * @brief     Provide functions for a first in first out queue
 * @details   ECE319K Lab 8.  
 * @author    change this to your names or look very silly
 * @date     change this to the last modification date or look very silly
 */

#ifndef __FIFO1_H__
#define __FIFO1_H__

#include <stdint.h>

/**
 * Initialize the ECE319K Lab 8 FIFO 
 * @param none
 * @return none
 * @see Fifo1_Put() Fifo1_Get()
 * @brief  Initialize FIFO
 */
void Fifo1_Init(void);

/**
 * Put character into ECE319K Lab 8 FIFO 
 * @param data is a new character to save
 * @return 0 for fail because full, 1 for success
 * @see Fifo1_Init() Fifo1_Get()
 * @brief Put FIFO
 */
uint32_t Fifo1_Put(char data);


/**
 * Get character from ECE319K Lab 8 FIFO 
 * @param none
 * @return 0 for fail because empty, nonzero is data
 * @see Fifo1_Init() Fifo1_Put()
 * @brief Get FIFO
 */
void Fifo1_Get(char *datapt);

uint32_t Fifo1_Size(void);

#endif //  __FIFO1_H__
/** @}*/
