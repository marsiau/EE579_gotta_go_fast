/*
 * BumpSens.h
 *
 *  Created on: 8 May 2019
 *      Author: marsiau
 */

#ifndef BUMPSENS_H_
#define BUMPSENS_H_

#include <msp430.h>
#include <stdbool.h>//For bool
#include <stdint.h>//For uintX_t

//----- Variable definitions -----
extern uint8_t BumpSwitch_flag;

//----- Function declarations -----

void Bump_init();

#endif /* BUMPSENS_H_ */
