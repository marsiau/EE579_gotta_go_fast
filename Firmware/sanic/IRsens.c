/*
 * IRsens.c
 *
 *  Created on: 7 Feb 2019
 *      Author: marsiau
 */
#include "IRsens.h"

//--------------- Variable definitions ---------------
const uint8_t ERR = 50;
bool calib_flag = false;
uint8_t IRSens_flag = 0; // 0b00 A5 A4 A3 A2 0 0 
uint16_t ADC_chnl;
uint16_t Vbat = 4500;

//Using FRAM to store calibration values
#ifdef __TI_COMPILER_VERSION__
    #pragma DATA_SECTION(white_lvl, ".TI.persistent")
    uint16_t white_lvl = {0};
#elif __IAR_SYSTEMS_ICC__
    __persistent uint16_t white_lvl;
#endif

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
                white_lvl = ADCMEM0+ERR;//-ERR
                SYSCFG0 |= PFWP;// Program FRAM write protected
                calib_flag = false;
            }
            else if(ADCMEM0 < white_lvl)
            {
                P5OUT ^= BIT5;
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
//                    case 0x6: // A6
//                        Vbat = (int)ADCMEM0 * 19 / 13;
//                        break;
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
    ADCCTL0 |= ADCENC | ADCSC; // Enable ADC conversion
}

void RTC_init(){ // Function used to automatically stop the car after 2:30 mins
  //RTCMOD = 1024/32768 * 4800 = 150 seconds
  RTCMOD = 4800-1;
  RTCCTL = RTCSS__XT1CLK | RTCSR | RTCPS__1024 | RTCIE; 
}
