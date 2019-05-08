#include "io430.h"

enum FwdRwd_flag {Stop,Forward,Reverse};
enum LR_flag {Neutral,Right,Left};

// Movement flags used to determine the status of the car
extern enum FwdRwd_flag drive_flag;
extern enum LR_flag steer_flag;

// Global variables where the cycle limit is stored
extern int FwdRwdCyclesLimit;
extern int RLCyclesLimit;

// Global variables where the current cycle is stored
extern int FwdRwdCycle;
extern int RLCycle;

void ACLKClockSetup();
void PWM_PeriodSetup(int Period);
void PWM_TimerSetup();
// CyclesLimit defines the movement duration
void Drive_FWD(int DutyCycle, int CyclesLimit); 
void Drive_RWD(int DutyCycle, int CyclesLimit);
void Steer_Left(int CyclesLimit);
void Steer_Right(int CyclesLimit);
void StopCar();