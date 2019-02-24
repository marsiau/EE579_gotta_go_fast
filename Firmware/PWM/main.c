
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
      PWM_DutyCycle(100); // Sets up the duty cycle of the PWM
    else if(ADCMEM0 > 795)
      PWM_DutyCycle(75);
    else if (ADCMEM0 > 591)
      PWM_DutyCycle(50);
    else if (ADCMEM0 > 387)
      PWM_DutyCycle(25);
    else
      PWM_DutyCycle(0);
    break;
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P1DIR |= BIT6 | BIT7;                     // P1.6 and P1.7 output
    P1SEL0 |= BIT6 | BIT7;                    // P1.6 and P1.7 options select
    
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    
    ACLKClockSetup();
    PWM_TimerSetup();           // Sets up the timer of the PWM
    PWM_PeriodSetup(100);       // Sets up the period of the PWM
    
    ADCsetup();
    
    __bis_SR_register(LPM3_bits|GIE);          // Enter LPM3
    __no_operation();                         // For debugger
}
