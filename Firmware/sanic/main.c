//#include <msp430.h> --included by irsense
#include "PWMsetup.h"
#include "IRsens.h"


bool initialised = false;
int DutyCycle = 99; // The Duty Cycle of the PWMs
int PWMPeriod = 200; // This defines the value of the CCR0

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
bool running = 0;
int scriptcount = -1;
int scriptselector = -1;

// Debugging variables
int MovementCyclesLimit = 41; // 1 second = 331 Cycles (DEBUGGING)
int MovementCyclesCounter = 0; // Counts the number of cycles (DEBUGGING)

// Duration of the movements is in seconds (DEBUGGING)
int ForwardCycleCounterLimit =  3 * 83;
int ReverseCycleCounterLimit = 2 * 83; //331 is 1s, 83 = 0.25ss
int LeftCycleCounterLimit = 3 * 83;
int RightCycleCounterLimit = 3 * 83;
// ---------------------------
//----- Function declarations -----

void Bump_init();

//----- Global variable declarations -----
extern uint8_t BumpSwitch_flag = 0x00;

//--------------- Interrupt routines ---------------
//----- Interrupt routine for GPIO -----
#pragma vector = PORT2_VECTOR
__interrupt void P1_interrupt_handler(void)
{
  //Debounce all at once
  __delay_cycles(10000);                      //Simple debouncing
  if((P2IN && 0x27) >= 0x1)
  {
    switch(__even_in_range(P2IV,P2IV_P2IFG7)){//Checks all pins on P1
      // Mising break to catch multiple events
    case P2IV_P2IFG0:
      BumpSwitch_flag |= 0x01; break; //Front left
    case P2IV_P2IFG1:
      BumpSwitch_flag |= 0x02; break;//Front
    case P2IV_P2IFG2:
      BumpSwitch_flag |= 0x04; break;//Front right
    case P2IV_P2IFG5:
      BumpSwitch_flag |= 0x20; break;//Back
    default:
      break;
    }
  }
  __no_operation();
  __bic_SR_register_on_exit(LPM3_bits);         //Exit LPM3
}

// RTC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(RTCIV,RTCIV_RTCIF))
    {
        case  RTCIV_NONE:   break;          // No interrupt
        case  RTCIV_RTCIF:                  // RTC Overflow
            P5OUT ^= BIT3;
            StopCar();
            scriptselector = -1;
            break;
        default: break;
    }
}

//--------------- Function declarations ---------------
void Bump_init()
{
  // Init GPIO P2.0-3
  P2DIR &= ~(BIT0 | BIT1 | BIT2 | BIT5); // Setup as input
  P2REN |=  (BIT0 | BIT1 | BIT2 | BIT5); // Enable pull up/down resistor
  P2OUT |=  (BIT0 | BIT1 | BIT2 | BIT5); // Select pull up
  P2IES |=  (BIT0 | BIT1 | BIT2 | BIT5); // Interrupt on high-to-low transition
  P2IE  |=  (BIT0 | BIT1 | BIT2 | BIT5); // Interrupt enabled
  P2IFG &= ~(BIT0 | BIT1 | BIT2 | BIT5); // Interrupt flag cleared
}



// Timer1 A0 interrupt service routine CC0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
  if(initialised);

  // This part is used to test that 331 cycles = 1 second by toggling the LED
  if(MovementCyclesCounter == MovementCyclesLimit){
    P5OUT ^= BIT4; // Led is toggled every 250ms (DEBUGGER)
    if(drive_flag == Forward) TA1CCR1 = DutyCycle; // Update duty cycle
    else if(drive_flag == Reverse) TA1CCR2 = DutyCycle; // Update duty cycle
    MovementCyclesCounter = 0;
  }
  else{
    MovementCyclesCounter++;// Used to toggle LED4.0 every second
  }

  //--------------------------------------------------
  // Duty Cycle selector
  if(running == 0) DutyCycle =  90;
  else{
    if(Vbat > 4400) DutyCycle = 5;
    else if(Vbat > 4300) DutyCycle = 75;
    else if(Vbat > 4200) DutyCycle = 80;
    else if(Vbat > 4100) DutyCycle = 85;
    else if(Vbat > 4000) DutyCycle = 90;
    else if(Vbat > 3900) DutyCycle = 95;
    else DutyCycle = 99;
  }

  //--------------------------------------------------
  //Forward-sensor Script
  switch(scriptselector){
  case 0: //Forward-Right Sensor
    switch(scriptcount){
    case 0:
      StopCar();
      __delay_cycles(10000);
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_RWD(DutyCycle,ReverseCycleCounterLimit);
      //sSteer_Right(ReverseCycleCounterLimit);
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 2:
      StopCar();
      __delay_cycles(10000);
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
    default: scriptcount = 0; scriptselector = 3; break;
    }
    break;
  case 1: //Forward-Left Sensor
    switch(scriptcount){
    case 0:
      StopCar();
      __delay_cycles(10000);
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_RWD(DutyCycle,ReverseCycleCounterLimit);
      //Steer_Left(ReverseCycleCounterLimit);
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    case 2:
      StopCar();
      __delay_cycles(10000);
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
    default: scriptcount = 0; scriptselector = 3; break;
    }
    break;
  case 2: //Back Sensor
    switch(scriptcount){
    case 0:
      StopCar();
      __delay_cycles(10000);
      running = 0;
      scriptcount++;
      break;
    case 1:
      if(!running)
        Drive_FWD(DutyCycle,41); //2secs
      running = 1;
      if(drive_flag == Stop){
        scriptcount++;
        running = 0;
      }
      break;
    default: running = 0; scriptselector = 2; scriptcount = 0; break;
    }
    break;
  case 3: //Compensate Steering state
    switch(scriptcount){
    case 0:
      __delay_cycles(10000);
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
    default: running = 0; scriptselector = 2; LCycles = 0; RCycles = 0; break;
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
      if((steer_flag == Left && drive_flag == Forward) || (steer_flag == Right && drive_flag == Reverse))
        LCycles++;
      else if((steer_flag == Right && drive_flag == Forward) || (steer_flag == Left && drive_flag == Reverse))
        RCycles++;
    }
  }
  else{
    if(steer_flag == Right);
    else if(steer_flag == Left);
    steer_flag = Neutral;                    // Set the flag to "Neutral"
    RLCycle = 0;                             // Reset the cycle counter
    P7OUT &= ~(BIT5);                        // Drive P7.5 Low (Right)
    P7OUT &= ~(BIT4);                        // Drive P7.4 Low (Left)
  }
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  P4DIR |= BIT0;     // P4.0 (FWD)
  P8DIR |= BIT3;     // P8.3 (RWD)

  P4SEL0 |= BIT0;     // Secondary function PWM output TA1.1
  P8SEL0 |= BIT3;     // Secondary function PWM output TA1.2

  P7DIR |= BIT5 | BIT4;                //P7.5(Left) P7.4(Right) output mode

  P7OUT &= ~(BIT4 | BIT5);             // Drive P7.4 Low (Right) P7.5 Low (Left)

  P4SEL0|= BIT1 | BIT2;                // P4.2~P4.1: crystal pins

  RTC_init();                           // RTC Init
  Bump_init();                          // Bump_sensor Init

  P5DIR |= BIT5 | BIT4 | BIT3; // (DEBUGGER) LEDs
  P5OUT &= ~(BIT3 | BIT5);
  ACLKClockSetup();           // Connects the external oscillator XT1 to ACLK
  PWM_TimerSetup();           // Sets up the timer of the PWM
  PWM_PeriodSetup(PWMPeriod); // Sets up the period of the PWM
  //initialised = false;
  //BumpSwitch_flag = 0x00;
  __enable_interrupt();
  while(!initialised)
  {
    if(BumpSwitch_flag & 0x20) //Back button pressed (Start)
    {
        //Do no calibration and just go, pull white_lvl from persistent memory
        initialised = true;
        //IRSens Init
        IR_init();
        IR_scan();
        BumpSwitch_flag = 0x00;
        scriptselector = 2; //Select Back Sensor Script(Go forward)
        scriptcount = 0; //Reset
        running = 0; //Not currently running a script (For movement loop logic)
    } else if(BumpSwitch_flag & 0x02) //Front button pressed - Calibrate
    {
      P5OUT |= BIT5;
      IR_calibrate();
      __delay_cycles(500000);
      P5OUT &= ~(BIT5);
      BumpSwitch_flag = 0x00;
      //scriptselector = 2; //Select Back Sensor Script(Go forward)
      //scriptcount = 0; //Reset
      //running = 0; //Not currently running a script (For movement loop logic)
    }
  }
  while(1)
  {
    if(IRSens_flag & 0x20 | BumpSwitch_flag & 0x20)//1.5 A5 Back (Highest Priority)
    {
      StopCar(); //Stop Immediately
      scriptselector = 2; //Select Back Sensor Script(Go forward)
      scriptcount = 0; //Reset
      running = 0; //Not currently running a script (For movement loop logic)
      IRSens_flag &= ~(0x20); // Reset IRSense Flag now sensor has been dealt with
      BumpSwitch_flag &= ~(0x20);
    }
    else if(IRSens_flag & 0x4 | BumpSwitch_flag & 0x01) //1.2 A2 Front-Left
    {
      StopCar(); //Stop Immediately
      scriptselector = 1; //Select Forward Left Sensor Script
      scriptcount = 0; //Reset
      running = 0; //Not currently running a script (For movement loop logic)
      IRSens_flag &= ~(0x4 | 0x8); // Reset IRSense Flag now sensor has been dealt with
      BumpSwitch_flag &= ~(0x01);
    }
    else if(IRSens_flag & 0x10 | BumpSwitch_flag & 0x04)//1.4 A4 Front-Right
    {
      StopCar(); //Stop Immediately
      scriptselector = 0; //Select Forward Right Sensor Script
      scriptcount = 0; //Reset
      running = 0; //Not currently running a script (For movement loop logic)
      IRSens_flag &= ~(0x10 | 0x8); // Reset IRSense Flag now sensor has been dealt with
      BumpSwitch_flag &= ~(0x04);
    }
    else if(IRSens_flag & 0x8 | BumpSwitch_flag & 0x02) //1.3 A3 Front (Lowest priority / if only front sensor)
    {
      StopCar(); //Stop Immediately
      if(LCycles >= RCycles)
          scriptselector = 1; //Either 0 or 1 for randomness (Forward right is 0, forward left sensor is 1)
      else
          scriptselector = 0;
      scriptcount = 0; //Reset
      running = 0; //Not currently running a script (For movement loop logic)
      IRSens_flag &= ~(0x8); // Reset IRSense Flag now sensor has been dealt with
      BumpSwitch_flag &= ~(0x02);
    }
    __bis_SR_register(LPM3_bits|GIE);          // Enter LPM3
    __no_operation();                         // For debugger
  }
}
