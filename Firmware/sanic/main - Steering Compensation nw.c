#include "io430.h"
#include "PWMsetup.h"

int DutyCycle = 75; // The Duty Cycle of the PWMs
int PWMPeriod = 100; // This defines the value of the CCR0 

// Movement flags used to determine the status of the car
enum FwdRwd_flag drive_flag = Stop;
enum LR_flag steer_flag = Neutral;

int FwdRwdCyclesLimit = 0; // Global variables where the cycle limit is stored
int RLCyclesLimit = 0;

int FwdRwdCycle = 0; // Global variables where the current cycle is stored
int RLCycle = 0;

//Steering tracker
int LCycles = 0;
int RCycles = 0;


//scripting variables
int running = 0;
int scriptcount = -1;
int scriptselector = -1;

// Debugging variables
int MovementCyclesLimit = 331; // 1 second = 331 Cycles (DEBUGGING)     
int MovementCyclesCounter = 0; // Counts the number of cycles (DEBUGGING)

// Duration of the movements is in seconds (DEBUGGING)
int ForwardCycleCounterLimit =  2 * 331;
int ReverseCycleCounterLimit = 2 * 331;
int LeftCycleCounterLimit = 1 * 331;
int RightCycleCounterLimit = 1 * 331;
// ---------------------------

#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  switch(__even_in_range(P2IV,P2IV_P2IFG7)) //P2.6
  {
  case P2IV_P2IFG6: //Back sensor
    __delay_cycles(5000);    // Debouncing, Wait and then check if P2.6 is still Low
    if(!(P2IN & BIT6)){
      StopCar();
      scriptselector = 2; //Select Back Sensor Script
      scriptcount = 0;
      running = 0;
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
  case P1IV_P1IFG2: //Forward-Facing Sensor //P1.2 
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.2 is still Low
    if(!(P1IN & BIT2)){
      StopCar();
      scriptselector = 0; //Either 0 or 1 for randomness (Forward right is 0, forward left sensor is 1)
      scriptcount = 0;
      running = 0;
      P1IFG &= ~BIT2;
      break;
    }
    
  case P1IV_P1IFG3: //It is SW1 // Forward-Left //P1.3
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.3 is still Low
    if(!(P1IN & BIT3)){
      StopCar();
      scriptselector = 1; //Select Forward Left Sensor Script
      scriptcount = 0;
      running = 0;
      P1IFG &= ~BIT3;
      break;
    }
  case P1IV_P1IFG4:  //It is SW2 // Forward-Right //P1.4
    __delay_cycles(5000);    // Debouncing, Wait and then check if P1.4 is still Low
    if(!(P1IN & BIT4)){
      StopCar();
      scriptselector = 0; //Select Forward Right Sensor
      scriptcount = 0;
      running = 0;
      P1IFG &= ~BIT4;      
      break;
    }
  } 
}

////ADC Interrupt Service Routine
//#pragma vector=ADC_VECTOR           
//__interrupt void ADC_ISR(void)
//{
//  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
//  {
//  case ADCIV_ADCIFG:              // Ready for a reading
//    DutyCycle = ADCMEM0 / 10; // Sets up the duty cycle of the PWM
//    // This updates the speed of the car as soon as a new Duty Cycle value is calculated
//    if(drive_flag == Forward) TA0CCR1 = DutyCycle; 
//    else if (drive_flag == Reverse) TA0CCR2 = DutyCycle;
//    break;
//  }
//}

// Timer A0 interrupt service routine CC0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{ 
  // This part is used to test that 331 cycles = 1 second by toggling the LED
  if(MovementCyclesCounter == MovementCyclesLimit){
    P4OUT ^= BIT3; // Led is toggled every 3.02ms (DEBUGGER)
    MovementCyclesCounter = 0;
  }
  else{
    MovementCyclesCounter++;// Used to toggle LED4.0 every second
  }
  //--------------------------------------------------
  //Forward-sensor Script
  switch(scriptselector){
  case 0: //Forward-Right Sensor
    switch(scriptcount){
    case 0:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_RWD(DutyCycle,ReverseCycleCounterLimit);
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 2:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 3:
      if(!running){
        Steer_Left(ReverseCycleCounterLimit);
        Drive_FWD(DutyCycle,ReverseCycleCounterLimit);
      }
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 4:
      if(!running){
        Steer_Right(ReverseCycleCounterLimit);
        Drive_FWD(DutyCycle,ReverseCycleCounterLimit);
      }
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    default: scriptselector = 3; break;
    } 
    break;
  case 1: //Forward-Left Sensor
    switch(scriptcount){
    case 0:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_RWD(DutyCycle,ReverseCycleCounterLimit);
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 2:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 3:
      if(!running){
        Steer_Right(ReverseCycleCounterLimit);
        Drive_FWD(DutyCycle,ReverseCycleCounterLimit);
      }
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 4:
      if(!running){
        Steer_Left(ReverseCycleCounterLimit);
        Drive_FWD(DutyCycle,ReverseCycleCounterLimit);
      }
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    default: scriptselector = 3; break;
    }
    break;
  case 2: //Back Sensor
        switch(scriptcount){
    case 0:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_FWD(DutyCycle,15000);
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    default: running = 0; scriptcount = 0; break;
    }
    break;
  case 3: //Compensate Steering
    switch(scriptcount){
    case 0:
      StopCar();
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running){
        if((LCycles - RCycles) > 0) {
          Steer_Right((LCycles - RCycles));
          Drive_FWD(DutyCycle,(LCycles - RCycles)); 
        }
        else {
          Steer_Left((RCycles - LCycles));
          Drive_FWD(DutyCycle,(RCycles - LCycles)); 
        }
      }
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    default: running = 0; scriptselector = 2; break;
    }
    break;
  }
  
  
  
  
  if(FwdRwdCycle < FwdRwdCyclesLimit){
    if(drive_flag != Stop){
      // If the cycle limit has not been reached and the status flag is not "Stop" then increment the Cycle counter
      FwdRwdCycle++; 
    }
  }
  else{ // When cycle limit is reached stop driving
    if(drive_flag == Forward) ;
    else if(drive_flag == Reverse) ;
    drive_flag = Stop;                       // Set the flag to "Stop"
    FwdRwdCycle = 0;                         // Reset the cycle counter
    TA1CCR1 = 0;                             // Set the "DutyCycle" to 0
    TA1CCR2 = 0;                             // Set the "DutyCycle" to 0
    TA1CCTL1 = OUTMOD_5;                     // CCR2 reset      P1.7/TA0.1
    TA1CCTL2 = OUTMOD_5;                     // CCR2 reset      P1.6/TA0.2
  }
  
  if(RLCycle < RLCyclesLimit){
    if(steer_flag != Neutral){
      // If the cycle limit has not been reached and the status flag is not "Neutral" then increment the Cycle counter
      RLCycle++;
      if(steer_flag == Left)
        LCycles++;
      else
        RCycles++;
    }
  }
  else{
    if(steer_flag == Right);
    else if(steer_flag == Left);
    steer_flag = Neutral;                    // Set the flag to "Neutral"                
    RLCycle = 0;                             // Reset the cycle counter   
    P7OUT &= ~(BIT5);                        // Drive P7.5 Low (Left)
    P7OUT &= ~(BIT4);                        // Drive P7.4 Low (Right)
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
    
  P4DIR |= BIT0;     // P4.0 (FWD)
  P8DIR |= BIT3;     // P8.3 (RWD)
  
  P4SEL0 |= BIT0;     // Secondary function PWM output TA1.1
  P8SEL0 |= BIT3;     // Secondary function PWM output TA1.2
  
  P7DIR |= BIT5 | BIT4;                //P7.5(Left) P7.4(Right) output mode
  
  P7OUT &= ~(BIT4 | BIT5);             // Drive P7.4 Low (Right) P7.5 Low (Left)
  
  P4SEL0|= BIT1 | BIT2;                // P4.2~P4.1: crystal pins
  
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
  P2IFG &= ~BIT6;  // Clear interrupt flag of P2.6
  
  P4DIR |= BIT3; // (DEBUGGER) Used to visualize the movement cycles during the run time
  
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
  
  ACLKClockSetup();           // Connects the external oscillator XT1 to ACLK
  PWM_TimerSetup();           // Sets up the timer of the PWM
  PWM_PeriodSetup(PWMPeriod);       // Sets up the period of the PWM
  
//  ADCsetup();                 // Set up the ADC configuration
  
  __bis_SR_register(LPM3_bits|GIE);          // Enter LPM3
  __no_operation();                         // For debugger
  
  
  
}
