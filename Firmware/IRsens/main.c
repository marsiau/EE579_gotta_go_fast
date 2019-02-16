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
	IR_scan();
	__enable_interrupt();

	void(1)
	{
	    //??
	    __bis_SR_register(LPM3_bits + GIE);     // Enter LPM3 w/interrupt
	    __no_operation();
	}

	return 0;
}
