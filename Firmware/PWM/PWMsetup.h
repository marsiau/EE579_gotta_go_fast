#include "io430.h"

extern void ACLKClockSetup();
extern void PWM_PeriodSetup(int Period);
extern void PWM_TimerSetup();
extern void MoveFWD(int DutyCycle);
extern void MoveRWD(int DutyCycle);
extern void Stop();
