/*
 * IRsens.h
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#ifndef IRSENS_H_
#define IRSENS_H_
#include "io430.h"
#include <stdint.h>//For uintX_t
#include <stdbool.h>//For bool
//#include <string.h>//For strcpy()

//----- Variable definitions -----
/*
 * Using FRAM to store calibration values
 * */
//Constants
extern const uint8_t ERR; //Define ADC error margin

#ifdef __TI_COMPILER_VERSION__
    #pragma PERSISTENT(white_lvl)
    extern uint16_t white_lvl;
#elif __IAR_SYSTEMS_ICC__
    __persistent extern uint16_t white_lvl;
#endif

// Global variables
extern bool calib_flag;//Calibration flag
extern uint8_t IRSens_flag;

//----- Function declarations -----
void IR_calibrate();//Calibrate the threshold value
void IR_init();//Setup ADC & timer for A9 (P8.1) only
void IR_scan();//Start ADC scan
void IR_stop();//
#endif /* IRSENS_H_ */