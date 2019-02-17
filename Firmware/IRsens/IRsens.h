/*
 * IRsens.h
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */

#ifndef IRSENS_H_
#define IRSENS_H_

#include <msp430.h>
#include <stdint.h>//For uintX_t
#include <stdbool.h>//For bool
//#include <string.h>//For strcpy()//could get away without it

//----- Variable definitions -----
/*
 * Using FRAM to store calibration values
 * */
//Constants
extern const uint8_t ERR; //Define ADC error margin

// Statically-initialised variables
#ifdef __TI_COMPILER_VERSION__
    #pragma PERSISTENT(white_lvl)
    extern uint16_t white_lvl;
    //#pragma PERSISTENT(ground_lvl)
    //extern uint16_t ground_lvl;
#elif __IAR_SYSTEMS_ICC__
    __persistent extern uint16_t white_lvl = 0;
    _//_persistent extern uint16_t ground_lvl = 0;
#endif

// Global variables
extern bool calib_flag;//Calibration flag


//----- Function declarations -----
void IR_init();
void ADC_GPIO_init();
void IR_calibrate();
void IR_scan();

#endif /* IRSENS_H_ */
