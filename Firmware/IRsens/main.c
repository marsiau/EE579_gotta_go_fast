#include <msp430.h>
//#include <stdint.h>
#include "IRsens.h"

//----- Interupt rutine for GPIO -----
#pragma vector = PORT1_VECTOR
__interrupt void P1_interrupt_handler(void)
{
  switch(__even_in_range(P1IV,P1IV_P1IFG7))//Checks all pins on P1
  {
  case P1IV_P1IFG2:                             //PIN2 - SW1
    __delay_cycles(5000);                      //Simple debauncing
    if(!(P1IN & BIT2))
    {
        P4OUT  ^= BIT0;
        __no_operation();
        IR_calibrate();
    }
    break;
  }
}
//----- main -----
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
	

	IR_init();
	//IR_scan();

/*
    //Setup up output LED pin
    P4DIR |= BIT0;
    P4OUT &= ~BIT0;
    //Setup the input button
    P1DIR &= ~BIT2;                             //P1.2 as input
    P1REN |= BIT2;                              //Enable pull up/down resistor on P1.2
    P1OUT |= BIT2;                              //Select pull up
    P1IES |= BIT2;                              //Interrupt on high-to-low transition
    P1IE  |= BIT2;                              //Interrupt enabled
    P1IFG &= ~BIT2;                             //P1.2 interrupt flag cleared
*//
    __enable_interrupt();
	while(1)
	{
	    //??
	    P4OUT  ^= BIT0;                         //Toggle the LED using XOR
	    __bis_SR_register(LPM3_bits + GIE);     // Enter LPM3 w/interrupt
	    __no_operation();
	}

	return 0;
}
