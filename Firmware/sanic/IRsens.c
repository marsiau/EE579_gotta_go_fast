/*
 * IRsens.c
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#include "IRsens.h"

//--------------- Variable definitions ---------------
const uint8_t ERR = 100;
bool calib_flag = false;
uint8_t IRSens_flag = 0; // 0b00 A5 A4 A3 A2 0 0 
uint16_t white_lvl;
uint16_t ADC_chnl;

//--------------- Interrupt routines ---------------
//----- Interrupt routine for ADC -----
//ADCMEM0 ranges 0 - 1023
#pragma vector=ADC_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG://ADCMEM0 overflow
            break;
        case ADCIV_ADCTOVIFG://ADC conversion-time overflow
            break;
        case ADCIV_ADCHIIFG: //window comparator interrupt
            break;
        case ADCIV_ADCLOIFG: //window comparator interrupt
            break;
        case ADCIV_ADCINIFG://window comparator interrupt
            break;
        case ADCIV_ADCIFG:
            if(calib_flag)
            {
                //Store the value
                SYSCFG0 &= ~PFWP;// Program FRAM write enable
                white_lvl = ADCMEM0-ERR;//-ERR
                SYSCFG0 |= PFWP;// Program FRAM write protected
                calib_flag = false;
            }
            else if(ADCMEM0 > white_lvl)
            {
                ADC_chnl = ADCMCTL0 && 0xF; //Extract ADCINCHx
                switch(ADC_chnl)
                {
                    case 0x2: // A2 / 1.2
                        IRSens_flag |= 0x4;
                        break;
                    case 0x3: // A3 / 1.3
                        IRSens_flag |= 0x8;
                        break;
                    case 0x4: // A4 / 1.4
                        IRSens_flag |= 0x10;
                        break;
                    case 0x5: // A5 / 1.5
                        IRSens_flag |= 0x20;
                        break;
                    //case 0x6: // A6
                        //store Vbat
                        //break;
                    default:
                        break;
                }
                //White line has been detected, brake out of the LP3 to deal with it
                __no_operation();
                __bic_SR_register_on_exit(LPM3_bits);         //Exit LPM3
            }
            break;
        default:
            break;
    }
}

//--------------- Function declarations ---------------
//----- Calibrate the white lvl -----
void IR_calibrate()
{
    /*
     * Assuming it's only done before IR_scan()
     * Blink lights when finished!
    */
    calib_flag = true;

    SYSCFG2 |= ADCPCTL3;// Configure ADC A3 pin
    ADCCTL0 |= ADCSHT_2 | ADCON;// ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHS_0;//ADCSC as ADC sample-and-hold source
    ADCCTL1 |= ADCSHP;// ADCCLK = MODOSC; sampling timer
    ADCCTL2 |= ADCRES;// 10-bit conversion results
    ADCMCTL0 |= ADCINCH_3;// A3 ADC input select; Vref=AVCC

    ADCIFG &= ~(0x01);//Clear interrupt flag
    ADCIE |= ADCIE0;// Enable ADC conv complete interrupt

    ADCCTL0 |= ADCENC;//Enable a single conversion
}
//----- Initialise ADC Scan -----
void IR_init()
{
    //Init GPIO pins used for ADC
    // Configure pins A2-A5 as ADC inputs
    SYSCFG2 |= ADCPCTL2 | ADCPCTL3 | ADCPCTL4 | ADCPCTL5;
    //Sample & hold time = 16 ADCCLK cycles | ADC on
    ADCCTL0 |= ADCSHT_2  | ADCON;
    //TA0 trigger | SAMPCON triggered by sampling timer | repeat-sequence-of-channels
    ADCCTL1 |= ADCSHS_1 | ADCSHP | ADCCONSEQ_3;
    //10 bit (10 clock cycle conversion time)
    ADCCTL2 |= ADCRES;
    //Configure ADC mux
    ADCMCTL0 |= ADCINCH_5;//1001b = A9, 000b = Vr+ = AVCC and Vr- = AVSS
    //Configure the interrupt
    ADCIFG &= ~(0x01);//Clear interrupt flag
    ADCIE |= ADCIE0;

    //----- Configure trigger ADC timer TA0 -----
    //32768 should be around 100Hz atm
    TA0CTL = TACLR;// Clear the timer.
    TA0CCR0 =  0x148;// Reset every
    TA0CCR1 =  0xEE;// Toggle OUT every to turn the ADC on next CCR0 int
    TA0CCTL1 |= OUTMOD_7;// TA1CCR1 toggle
    TA0CTL |= TASSEL_1 | ID_0 | MC_0 | TACLR;// ACLK | no clock division | stop | clear
}
//----- Start scanning -----
void IR_scan()
{
    TA0CTL |= MC_1;// ACLK, up mode
    ADCCTL0 |= ADCENC;// Enable conversion
}
