#include "ADC_Timers_Setup.h"

void ADCsetup(){
  
  //1. Configuring the pin
  SYSCFG2 |= ADCPCTL9; // Turn on analogue pin A9 (so it is not a GPIO).
  
  //2. Configuring CLK Source, operation mode
  ADCCTL0 |= ADCSHT_5 | ADCON; //128 Clk cycles in the sampling period for ADC | ADC ON(only modify when ADCENC=0)
  ADCCTL1 |= ADCSHS_2 | ADCSHP| ADCCONSEQ_2 | ADCSSEL_1;//SHS is Timer trigger 1 (TA1.1B) bit and CLK source is ACLK with single-channel repeated-conversion  
  ADCCTL2 |= ADCRES;//10 bit resolution
  
  //3. Configuring ADC MUX and positive and negative references
  ADCMCTL0 |= ADCINCH_9;//Analog Input 9 selected as input channel
  
  //4. Configuring the interrupt
  ADCIE |= ADCIE0;//Interrupt Enable  
  ADCCTL0 |= ADCENC ; //ADC Enable Conversion
  
  //Timer TA1.1B Configuration for ADC Trigger Signal
  //Initialize TAxCCRn
  TA1CCR0 |= 0x80; //count up to 2048 with a frequency of 32768 Hz which means 1/16s
  TA1CCR1 |= 0x20; //count up to 1024 with a frequency of 32768 Hz which means 1/32s

  //Apply desired config to TAxIV, TAIDEX and TAxCCTLn
  TA1CCTL1 |= OUTMOD_4 ; // Interrupt enabled | By default Capture Mode
  
  //Apply desired config to TAxCTL including to MC bits
  TA1CTL |= TASSEL__ACLK | MC_1 | TACLR;// Clock Selected ACLK 32 kHz | Input divider/1 |Up mode | Interrupt enabled
}