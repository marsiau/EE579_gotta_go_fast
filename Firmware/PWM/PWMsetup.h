#include "io430.h"

typedef int bool;
#define true 1
#define false 0

extern void ACLKClockSetup();
extern void PWM_PeriodSetup(int Period);
extern void PWM_TimerSetup();
extern bool MoveFWD(int DutyCycle, int Cycles, int CyclesLimit);
extern bool MoveRWD(int DutyCycle, int Cycles, int CyclesLimit);
extern bool MoveLeft(int Cycles, int CyclesLimit);
extern bool MoveRight(int Cycles, int CyclesLimit);
extern void Stop();
