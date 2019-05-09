/*
 * BumpSens.c
 *
 *  Created on: 8 May 2019
 *      Author: marsiau
 */
#include "BumpSens.h"

//--------------- Variable definitions ---------------
uint8_t BumpSwitch_flag = 0;

//--------------- Interrupt routines ---------------
//----- Interrupt routine for GPIO -----
#pragma vector = PORT2_VECTOR
__interrupt void P1_interrupt_handler(void)
{
  switch(__even_in_range(P2IV,P2IV_P2IFG7))//Checks all pins on P1
  {
  case P2IV_P2IFG7:
    __delay_cycles(5000);                      //Simple debauncing
    if(!(P2IN & BIT2))
    {
        P4OUT  ^= BIT0;
        __no_operation();
    }
    break;
  }
}

//--------------- Function declarations ---------------
void Bump_init()
{
    // Init GPIO P2.0-3
    P2DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3); // Setup as input
    P2REN |=  (BIT0 | BIT1 | BIT2 | BIT3); // Enable pull up/down resistor
    P2OUT |=  (BIT0 | BIT1 | BIT2 | BIT3); // Select pull up
    P2IES |=  (BIT0 | BIT1 | BIT2 | BIT3); // Interrupt on high-to-low transition
    P2IE  |=  (BIT0 | BIT1 | BIT2 | BIT3); // Interrupt enabled
    P2IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3); // Interrupt flag cleared
}
