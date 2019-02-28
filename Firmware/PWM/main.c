#include "io430.h"
#include "PWMsetup.h"
#include "ADC_Timers_Setup.h"


typedef int bool;
#define true 1
#define false 0

bool FWD_flag = 0;
bool RWD_flag = 0;
int DutyCycle=0;

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
  switch(__even_in_range(P1IV,P1IV_P1IFG7)) 
  {
  case P1IV_P1IFG2: //It is SW1
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.2 is still Low
    if(!(P1IN & BIT2)){
      TA0CTL |= TACLR;
      Stop();
      FWD_flag = 0;
      RWD_flag = 0;
      P1IFG &= ~BIT2;
      break;
    }
  case P1IV_P1IFG3: //It is SW1
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.3 is still Low
    if(!(P1IN & BIT3)){
      TA0CTL |= TACLR;
      FWD_flag = 1;
      RWD_flag = 0;
      P1IFG &= ~BIT3;
      break;
    }
    
  case P1IV_P1IFG4:  //It is SW2
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.4 is still Low
    if(!(P1IN & BIT4)){
      TA0CTL |= TACLR;
      FWD_flag = 0;
      RWD_flag = 1;
      P1IFG &= ~BIT4;       //this issue will appear when SW2 is pressed at LED7
      break;
    }
  } 
}

//ADC Interrupt Service Routine
#pragma vector=ADC_VECTOR           
__interrupt void ADC_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
  case ADCIV_ADCIFG:              // Ready for a reading
    DutyCycle = ADCMEM0 / 10; // Sets up the duty cycle of the PWM
    if(FWD_flag == 1) MoveFWD(DutyCycle);
      else if(RWD_flag == 1) MoveRWD(DutyCycle);
    break;
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P1DIR |= BIT6 | BIT7;                     // P1.6 and P1.7 output
    P1SEL0|= BIT6 | BIT7;                    // P1.6 and P1.7 options select
    
      // Configure SW1 and SW2 and P1.2
    P1OUT |= BIT2 | BIT3 | BIT4;   // Configure P1.2|P1.3|P1.4 as pulled-up
    P1REN |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 pull-up register enable
    P1IES |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 Hi/Low edge transition
    P1IE  |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 interrupt enabled
    P1IFG &= ~(BIT2 | BIT3 | BIT4); // Clear interrupt flag of P1.2|P1.3|P1.4
      
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    
    ACLKClockSetup();
    PWM_TimerSetup();           // Sets up the timer of the PWM
    PWM_PeriodSetup(100);       // Sets up the period of the PWM
    
    ADCsetup();                 // Set up the ADC configuration
    
    __bis_SR_register(LPM3_bits|GIE);          // Enter LPM3
    __no_operation();                         // For debugger
}