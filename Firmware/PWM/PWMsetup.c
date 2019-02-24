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
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;  // ACLK, up mode, clear TAR
    TA0CCTL1 = OUTMOD_7;                     // CCR1 reset/set  P1.7/TA0.1
    TA0CCTL2 = OUTMOD_3;                     // CCR2 reset/set  P1.6/TA0.2
}

void PWM_PeriodSetup(int Period){
    TA0CCR0 = Period-1;                         // PWM Period
}
extern void PWM_DutyCycle(int Period){
    TA0CCR1 = Period; 
    TA0CCR2 = Period;
}