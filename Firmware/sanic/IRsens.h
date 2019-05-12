/*
 * IRsens.h
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#ifndef IRSENS_H
#define IRSENS_H
#include <msp430.h>
#include <stdint.h>//For uintX_t
#include <stdbool.h>//For bool
//#include <string.h>//For strcpy()

//----- Variable definitions -----
//Constants
extern const uint8_t ERR; //Define ADC error margin

// Global variables
extern bool calib_flag;//Calibration flag
extern uint8_t IRSens_flag;
extern uint16_t Vbat;
//----- Function declarations -----
void IR_calibrate();//Calibrate the threshold value
void IR_init();//Setup ADC & timer for A9 (P8.1) only
void IR_scan();//Start ADC scan
void IR_stop();//
void RTC_init();
#endif /* _IRSENS_H_ */
