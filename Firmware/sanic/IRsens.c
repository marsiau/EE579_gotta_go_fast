/*
 * IRsens.c
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#include "IRsens.h"

//--------------- Variable definitions ---------------
const uint8_t ERR = 10;
bool calib_flag = false;
uint8_t IRSens_flag = 0; // 0b00 A5 A4 A3 A2 0 0 
uint16_t white_lvl;
uint16_t ADC_chnl;
uint16_t Vbat = 0;

//--------------- Interrupt routines ---------------
//----- Interrupt routine for ADC -----
//ADCMEM0 ranges 0 - 1023
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
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
        {
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
                //ADC_chnl = (int)(ADCMCTL0 && 0xF); //Extract ADCINCHx
                //ADC_chnl = (uint16_t) (ADCMCTL0 & 0xF); //Extract ADCINCHx
                ADC_chnl = (uint16_t) (ADCMCTL0 & 0xF); //Extract ADCINCHx
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
                      //  Vbat = (int)ADCMEM0 * 19 / 13;
                        //break;
                    default:
                        break;
                }
                //White line has been detected, brake out of the LP3 to deal with it
                __no_operation();
                __bic_SR_register_on_exit(LPM3_bits);         //Exit LPM3
            }
            ADCIFG = 0;
            break;
        }
        default:
            break;
    }
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
            //ADCCTL0 |= ADCSC;// Enable conversion
            break;
        default: break;
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

    ADCCTL0 |= ADCENC | ADCSC;//Enable a single conversion
}
//----- Initialise ADC Scan -----
void IR_init()
{
    /*
    //Init GPIO pins used for ADC
    // Configure pins A2-A5 as ADC inputs
    SYSCFG2 |= ADCPCTL2 | ADCPCTL3 | ADCPCTL4 | ADCPCTL5 | ADCPCTL6;
    //Sample & hold time = 16 ADCCLK cycles | ADC on
    ADCCTL0 |= ADCSHT_2  | ADCMSC | ADCON;
    //TA0 trigger | SAMPCON triggered by sampling timer | repeat-sequence-of-channels
    ADCCTL1 |= ADCSHS_1 | ADCSHP | ADCCONSEQ_2 | ADCSSEL_1;
    //10 bit (10 clock cycle conversion time)
    ADCCTL2 |= ADCRES;
    //Configure ADC mux
    ADCMCTL0 |= ADCINCH_5;//1001b = A9, 000b = Vr+ = AVCC and Vr- = AVSS
    //Configure the interrupt
    ADCIFG &= ~(0x01);//Clear interrupt flag
    ADCIE |= ADCIE0;
    */
     /*
     // with RTC
    ADCCTL0 &= ~ADCENC; //Turn off ADC
    //Init GPIO pins used for ADC
    // Configure pins A2-A5 as ADC inputs
    SYSCFG2 |= ADCPCTL2 | ADCPCTL3 | ADCPCTL4 | ADCPCTL5 ;//| ADCPCTL6;
    //Sample & hold time = 16 ADCCLK cycles | ADC on
    ADCCTL0 = ADCSHT_2 | ADCON;
    //TA0 trigger | SAMPCON triggered by sampling timer | repeat-sequence-of-channels
    ADCCTL1 = ADCSHS_1 | ADCSHP | ADCCONSEQ_3 | ADCSSEL_1;
    //10 bit (10 clock cycle conversion time)
    ADCCTL2 = ADCRES;
    //Configure ADC mux
    ADCMCTL0 |= ADCINCH_5;//1001b = A9, 000b = Vr+ = AVCC and Vr- = AVSS
    //Configure the interrupt
    ADCIFG &= ~(0x01);//Clear interrupt flag
    ADCIE = ADCIE0;
    */
/*
 * ok zajibal...
 * To configure the converter to perform successive conversions automatically and as quickly as possible, a
multiple sample and convert function is available. When ADCMSC = 1, CONSEQx > 0, and the sample
timer is used, the first rising edge of the SHI signal triggers the first conversion. Successive conversions
are triggered automatically as soon as the prior conversion is completed. Additional rising edges on SHI
are ignored until the sequence is completed in the single-sequence mode, or until the ADCENC bit is
toggled in repeat-single-channel or repeated-sequence modes. The function of the ADCENC bit is
unchanged when using the ADCMSC bit.
 */
    ADCCTL0 &= ~ADCENC; //Turn off ADC
    //Init GPIO pins used for ADC
    // Configure pins A2-A5 as ADC inputs
    SYSCFG2 |= ADCPCTL2 | ADCPCTL3 | ADCPCTL4 | ADCPCTL5 ;//| ADCPCTL6;
    //Sample & hold time = 16 ADCCLK cycles | ADC on
    ADCCTL0 = ADCSHT_2 | ADCMSC | ADCON;
    //TA0 trigger | SAMPCON triggered by sampling timer | repeat-sequence-of-channels
    ADCCTL1 = ADCSHS_0 | ADCSHP | ADCCONSEQ_3 | ADCSSEL_1;
    //10 bit (10 clock cycle conversion time)
    ADCCTL2 = ADCRES;
    //Configure ADC mux
    ADCMCTL0 |= ADCINCH_5;//1001b = A9, 000b = Vr+ = AVCC and Vr- = AVSS
    //Configure the interrupt
    ADCIFG &= ~(0x01);//Clear interrupt flag
    ADCIE = ADCIE0;




}
//----- Start scanning -----
void IR_scan()
{
    //----- Configure RTC to trigger ADC  -----
    // Using RTC as we need TA1 for PWM, and there is not connection between TA0 and ADC
    // Initialize RTC
    // RTC count re-load compare value at 32.
    // 1/32768 * 328 = x sec.
    RTCMOD = 0x400;
    RTCCTL = RTCSS__XT1CLK | RTCSR | RTCPS_2; // | RTCIE;
    ADCCTL0 |= ADCENC | ADCSC; // Enable ADC conversion
}
