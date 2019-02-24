#include "PWMsetup.h"

void ClockSetup(){
    // Configure clock
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