#include "io430.h"
//#include <msp430.h>

extern void ACLKClockSetup();
extern void PWM_PeriodSetup(int Period);
extern void PWM_TimerSetup();
extern void MoveFWD(int DutyCycle);
extern void MoveRWD(int DutyCycle);
extern void MoveLeft();
extern void MoveRight();
extern void Stop();
