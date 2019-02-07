/*
 * IRsens.h
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */

#ifndef IRSENS_H_
#define IRSENS_H_

#include <msp430.h>
#include <stdint.h>                             //For uintX_t

//----- Variable definitions -----
// Statically-initialized variable
#ifdef __TI_COMPILER_VERSION__
    #pragma PERSISTENT(white_lvl)
    extern uint16_t white_lvl;
    #pragma PERSISTENT(ground_lvl)
    extern uint16_t ground_lvl;
#elif __IAR_SYSTEMS_ICC__
    __persistent extern uint16_t white_lvl = 0;
    __persistent extern uint16_t ground_lvl = 0;
#endif

//----- Function declarations -----
void IR_init();
void IR_calibrate();
void IR_scan();

#endif /* IRSENS_H_ */
