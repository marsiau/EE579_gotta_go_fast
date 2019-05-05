#include "io430.h"
#include "PWMsetup.h"
#include "ADC_Timers_Setup.h"

typedef int bool;
#define true 1
#define false 0

bool FWD_flag = 0;
bool RWD_flag = 0;
bool Left_flag = 0;
bool Right_flag = 0;

int DutyCycle = 0;

int PWMPeriod = 100;

int MovementCyclesLimit = 331; // 1 s = 331 Cycles
int MovementCyclesCounter = 0; // Counts the number of cycles

// Counts how many seconds have passed while moving in X direction
int ForwardCycleCounter = 0;
int ReverseCycleCounter = 0;
int LeftCycleCounter = 0;
int RightCycleCounter = 0;

// Duration of the movements is in seconds
int ForwardCycleCounterLimit = 5;
int ReverseCycleCounterLimit = 10;
int LeftCycleCounterLimit = 3;
int RightCycleCounterLimit = 2;

#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  switch(__even_in_range(P2IV,P2IV_P2IFG7)) 
  {
  case P2IV_P2IFG6: //Reverse
    __delay_cycles(5000);    // Debouncing, Wait and then check if P2.6 is still Low
    if(!(P2IN & BIT6)){
      ReverseCycleCounter=0;
      FWD_flag = 0;
      RWD_flag = 1;
      P2IFG &= ~BIT6;      
      break;
    }
  }
}
#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
  switch(__even_in_range(P1IV,P1IV_P1IFG7)) 
  {
  case P1IV_P1IFG2: //Forward
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.2 is still Low
    if(!(P1IN & BIT2)){
      ForwardCycleCounter=0;
      FWD_flag = 1;
      RWD_flag = 0;
      P1IFG &= ~BIT2;
      break;
    }
    
  case P1IV_P1IFG3: //It is SW1 // Right
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.3 is still Low
    if(!(P1IN & BIT3)){
      RightCycleCounter=0;
      Right_flag = 1;
      Left_flag = 0 ;
      P1IFG &= ~BIT3;
      break;
    }
  case P1IV_P1IFG4:  //It is SW2 // Left
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.4 is still Low
    if(!(P1IN & BIT4)){
      LeftCycleCounter=0;
      Right_flag = 0;
      Left_flag = 1;
      P1IFG &= ~BIT4;      
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
    break;
  }
}

// Timer A0 interrupt service routine CC0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
  if(MovementCyclesCounter == MovementCyclesLimit){
    ForwardCycleCounter++;
    ReverseCycleCounter++;
    LeftCycleCounter++;
    RightCycleCounter++;
    
    // These IF statements check if the duration limit of a move has been 
    // reached and stop that move
    if(ForwardCycleCounter==ForwardCycleCounterLimit){
      ForwardCycleCounter = 0;
      FWD_flag = 0;
      TA0CCR1 = 0;
      TA0CCTL1 = OUTMOD_5;
    }
    if(ReverseCycleCounter==ReverseCycleCounterLimit){
      ReverseCycleCounter = 0;
      RWD_flag = 0;
      TA0CCR2 = 0; 
      TA0CCTL2 = OUTMOD_5;
    }
    if(LeftCycleCounter==LeftCycleCounterLimit){
      LeftCycleCounter = 0;
      Left_flag = 0;
      P1OUT &= ~(BIT5);
    }
    if(RightCycleCounter==RightCycleCounterLimit){
      RightCycleCounter = 0;
      Right_flag = 0;
      P5OUT &= ~(BIT0);
    }
    P4OUT ^= BIT0;
    MovementCyclesCounter = 0;
  }
  else {
    if(FWD_flag == 1) MoveFWD(DutyCycle);
    else if(RWD_flag == 1) MoveRWD(DutyCycle);
    if(Left_flag == 1) MoveLeft();
    else if(Right_flag == 1) MoveRight();
    MovementCyclesCounter++;
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
  
  P1DIR |= BIT0| BIT5 | BIT6 | BIT7; // P1.6(RWD) and P1.7 (FWD) P1.5(Left) output
  P1SEL0|= BIT6 | BIT7;               // P1.7 options select
  
  P5DIR |= BIT0;                      //P5.0(Right) output mode
  
  P1OUT &= ~BIT5;
  P5OUT &= ~BIT0;
  
  P4SEL0|= BIT1 | BIT2;                   // P4.2~P4.1: crystal pins
  
  // Configure SW1 and SW2 and P1.2
  P1OUT |= BIT2 | BIT3 | BIT4;   // Configure P1.2|P1.3|P1.4 as pulled-up
  P1REN |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 pull-up register enable
  P1IES |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 Hi/Low edge transition
  P1IE  |= BIT2 | BIT3 | BIT4;   // P1.2|P1.3|P1.4 interrupt enabled
  P1IFG &= ~(BIT2 | BIT3 | BIT4); // Clear interrupt flag of P1.2|P1.3|P1.4
  
  P2OUT |= BIT6;   // Configure P2.6 as pulled-up
  P2REN |= BIT6;   // P2.6 pull-up register enable
  P2IES |= BIT6;   // P2.6 Hi/Low edge transition
  P2IE  |= BIT6;   // P2.6 interrupt enabled
  P2IFG &= ~BIT6; // Clear interrupt flag of P2.6
    
  P4DIR |= BIT0; // Used to visualize the movement cycles during the run time
    
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
  
  ACLKClockSetup();
  PWM_TimerSetup();           // Sets up the timer of the PWM
  PWM_PeriodSetup(PWMPeriod);       // Sets up the period of the PWM
  
  ADCsetup();                 // Set up the ADC configuration
  
  __bis_SR_register(LPM3_bits|GIE);          // Enter LPM3
  __no_operation();                         // For debugger
}