
#include "io430.h"
#include "PWMsetup.h"
#include "ADC_Timers_Setup.h"


//ADC Interrupt Service Routine
#pragma vector=ADC_VECTOR           
__interrupt void ADC_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
  case ADCIV_ADCIFG:              // Ready for a reading
    if(ADCMEM0 > 1000)
      TA0CCR1 = 99; //Update TA0CCR0
    else if(ADCMEM0 > 795)
      TA0CCR1 = 75;
    else if (ADCMEM0 > 591)
      TA0CCR1 = 50;
    else if (ADCMEM0 > 387)
      TA0CCR1 = 25;
    else
      TA0CCR1 = 0;
    break;
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P1DIR |= BIT6 | BIT7;                     // P1.6 and P1.7 output
    P1SEL0 |= BIT6 | BIT7;                    // P1.6 and P1.7 options select
    
    P1DIR |= BIT0;
    
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    ClockSetup();
    TA0CCR0 = 100-1;                         // PWM Period
    TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA0CCR1 = 75;                            // CCR1 PWM duty cycle
   // TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
   // TA0CCR2 = 250;                            // CCR2 PWM duty cycle
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;  // SMCLK, up mode, clear TAR
    
    
    ADCsetup();
//    __bis_SR_register(LPM3_bits|GIE);             // Enter LPM0
    __no_operation();                         // For debugger
      __enable_interrupt();
    while(1){
      if (ADCMEM0 < 0x155)
        P1OUT &= ~BIT0;                                   // Clear P1.0 LED off
      else
        P1OUT |= BIT0;     }
}
