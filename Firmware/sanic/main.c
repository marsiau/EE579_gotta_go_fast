#include <msp430.h> 
/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;// Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    //Setup CLK

    __enable_interrupt();
    while(1)
    {
        __bis_SR_register(LPM3_bits + GIE);// Enter LPM3 w/interrupt
        __no_operation();
    }
    return 0;
}
