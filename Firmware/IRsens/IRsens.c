/*
 * IRsens.c
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#include "IRsens.h"

//--------------- Variable definitions ---------------
uint16_t white_lvl = 0;
uint16_t ground_lvl = 0;
//--------------- Interrupt routines ---------------
//----- Interrupt routine for ADC -----
//ADCMEM0 ranges 0 - 1023
#pragma vector=ADC_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
  case ADCIV_ADCIFG:
      //ADCMEM0
      __bic_SR_register_on_exit(LPM3_bits);         //Exit LPM3
      break;
  }
}

//--------------- Function declarations ---------------
void IR_init()
{

}

void IR_calibrate()
{
    SYSCFG0 &= ~PFWP;                   //Program FRAM write enable
    //strncpy(PHNR, (pstr+6), 13);
    SYSCFG0 |= PFWP;                    //Program FRAM write protected
}
void IR_scan()
{
    //----- Configure ADC -----
    SYSCFG2 |= ADCPCTL9;                                    //Configure pin 9 as ADC in
    //0010b = 16 ADCCLK cycles | | turn on ADC
    ADCCTL0 |= ADCSHT_2  | ADCON;                           //Sample & hold time = 16 ADCCLK cycles, ADC on
    //TA1.1 trigger | SAMPCON triggered by sampling timer | 10b = Repeat-single-channel
    ADCCTL1 |= ADCSHS_2 | ADCSHP | ADCCONSEQ_2;
    //10 bit (10 clock cycle conversion time)
    ADCCTL2 |= ADCRES;
        //Configure ADC mux
    ADCMCTL0 |= ADCINCH_9;                                  //1001b = A9, 000b = Vr+ = AVCC and Vr- = AVSS
        //Configure the interrupt
    ADCIFG &= ~(0x01);                                      //Clear interrupt flag
    ADCIE |= ADCIE0;

    //----- Configure trigger ADC timer TA1.1 -----
    //The count-to value, 32768/8 = 4096 = 0x1000
    TA1CTL = TACLR;                                         //Clear the timer.
    TA1CCR0 =  0x1000;                                      //Reset every 0x1000
    TA1CCR1 =  0x800;                                       //Toggle OUT every 0x800 to turn the ADC on 8 times/s
    TA1CCTL1 = OUTMOD_7;                                    //TA1CCR1 toggle
    TA1CTL = TASSEL_1 | MC_1 | TACLR;                       //ACLK, up mode

    ADCCTL0 |= ADCENC;                                      //Enable conversion
}
