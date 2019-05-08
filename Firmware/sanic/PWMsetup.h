#include "io430.h"

typedef int bool;
#define true 1
#define false 0


//extern int Cycles;
void ACLKClockSetup();
void PWM_PeriodSetup(int Period);
void PWM_TimerSetup();
bool MoveFWD(int DutyCycle, int Cycles, int CyclesLimit);
bool MoveRWD(int DutyCycle, int Cycles, int CyclesLimit);
bool MoveLeft(int Cycles, int CyclesLimit);
bool MoveRight(int Cycles, int CyclesLimit);
void Stop();
