#include <msp430.h>
//#include <stdint.h>
#include "IRsens.h"

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
	



	__enable_interrupt();
	return 0;
}
