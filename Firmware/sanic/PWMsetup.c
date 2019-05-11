#include "PWMsetup.h"

void ACLKClockSetup(){
  // Connects the external oscillator XT1 to ACLK
  __bis_SR_register(SCG0);                          // disable FLL
  CSCTL3 |= SELREF__REFOCLK;                        // Set REFOCLK as FLL reference source
  CSCTL0 = 0;                                       // clear DCO and MOD registers
  CSCTL1 &= ~(DCORSEL_7);                           // Clear DCO frequency select bits first
  CSCTL1 |= DCORSEL_3;                              // Set DCOCLK = 8MHz
  CSCTL2 = FLLD_1 + 121;                            // FLLD = 1, DCODIV = 4MHz
  __delay_cycles(3);
  __bic_SR_register(SCG0);                          // enable FLL
  while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));        // Poll until FLL is locked
  CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK;         // set ACLK = XT1 = 32768Hz, DCOCLK as MCLK and SMCLK source
  CSCTL5 |= DIVM0 | DIVS0;                          // SMCLK = MCLK = DCODIV = 1MHz, by default
}
void PWM_TimerSetup(){
  TA1CTL = TASSEL__ACLK | MC__UP | TACLR;  // ACLK, UP mode, clear TAR
  TA1CCTL0 |= CCIE;                        // Enable interrupt when CCR0 value is reached
  TA1CCTL1 = OUTMOD_7;                     // CCR1 reset/set  P4.0/TA1.1 FWD
  TA1CCTL2 = OUTMOD_7;                     // CCR2 reset/set  P8.3/TA1.2 RWD
}

void PWM_PeriodSetup(int Period){
  TA1CCR0 = Period-1;                      // PWM Period setup
}

void StopCar(){		                   // Stops the car
  TA1CCR1 = 0;                             // Set the "DutyCycle" to 0        
  TA1CCR2 = 0;                             // Set the "DutyCycle" to 0
  TA1CCTL1 = OUTMOD_5;                     // CCR1 reset   P4.0/TA1.1 FWD
  TA1CCTL2 = OUTMOD_5;                     // CCR2 reset   P8.3/TA1.2 RWD
  TA1CTL |= TACLR;                        
  P7OUT &= ~BIT4;                          //P7.4 Right
  P7OUT &= ~BIT5;                          //P7.5 Left
  drive_flag = Stop;
  steer_flag = Neutral;
}

void Drive_RWD(int DutyCycle, int CyclesLimit){ // Moves the car forward with a duty cycle given by "DutyCycle"
  drive_flag = Reverse;                    // Overwrites the global drive flag
  FwdRwdCyclesLimit = CyclesLimit;         // Overwrites the global cycle limit
  FwdRwdCycle = 0;                         // Resets the global cycle counter
  TA1CCR1 = DutyCycle;                     
  TA1CCR2 = 0;                             // Set the "DutyCycle" to 0
  TA1CCTL1 = OUTMOD_7;                     // CCR1 reset/set  P4.0/TA1.1 FWD
  TA1CCTL2 = OUTMOD_5;                     // CCR2 reset      P8.3/TA1.2 RWD
}

void Drive_FWD(int DutyCycle, int CyclesLimit){    // Moves the car backwards with a duty cycle given by "DutyCycle"
  drive_flag = Forward;                    // Overwrites the global drive flag
  FwdRwdCyclesLimit = CyclesLimit;         // Overwrites the global cycle limit
  FwdRwdCycle = 0;                         // Resets the global cycle counter
  TA1CCR1 = 0;                             // Set the "DutyCycle" to 0
  TA1CCR2 = DutyCycle;
  TA1CCTL1 = OUTMOD_5;                     // CCR1 reset      P4.0/TA1.1 FWD
  TA1CCTL2 = OUTMOD_7;                     // CCR2 reset/set  P8.3/TA1.2 RWD
}

void Steer_Left(int CyclesLimit){	    // Steers the car Left
  steer_flag = Left;                        // Overwrites the global steer flag
  RLCyclesLimit = CyclesLimit;              // Overwrites the global cycle limit
  RLCycle = 0;                              // Resets the global cycle counter  
  P7OUT |=  BIT4;
  P7OUT &= ~BIT5;
}

void Steer_Right(int CyclesLimit){	    // Steers the car Right
  steer_flag = Right;                       // Overwrites the global steer flag
  RLCyclesLimit = CyclesLimit;              // Overwrites the global cycle limit
  RLCycle = 0;                              // Resets the global cycle counter
  P7OUT &= ~BIT4;
  P7OUT |=  BIT5;
}
