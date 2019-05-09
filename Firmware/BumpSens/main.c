/**
 * main.c
 */
#include <msp430.h>
#include <stdbool.h>//For bool
#include <stdint.h>//For uintX_t

//----- Function declarations -----

void Bump_init();

//----- Global variable declarations -----
extern uint8_t BumpSwitch_flag = 0;

//--------------- Interrupt routines ---------------
//----- Interrupt routine for GPIO -----
#pragma vector = PORT2_VECTOR
__interrupt void P1_interrupt_handler(void)
{
    //Debounce all at once
    __delay_cycles(5000);                      //Simple debouncing
    if((P2IN && 0x27) > 1)
    {
        switch(__even_in_range(P2IV,P2IV_P2IFG7))//Checks all pins on P1
        // Mising break to catch multiple events
            case P2IV_P2IFG0:
                BumpSwitch_flag |= 0x01;
            case P2IV_P2IFG1:
                BumpSwitch_flag |= 0x02;
            case P2IV_P2IFG2:
                BumpSwitch_flag |= 0x04;
            case P2IV_P2IFG5:
                BumpSwitch_flag |= 0x20;
            default:
                break;
    }
}

//----- main -----

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	return 0;
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
