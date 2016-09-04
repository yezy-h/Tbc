#ifndef __PID_H__
#define __PID_H__
#include "stm32f10x.h"
#include "includes.h"

#define CURRENT_TIME    (OSTimeGet() * 1000 / OS_TICKS_PER_SEC) 

#define AUTOMATIC       1
#define MANUAL          0
#define DIRECT          0
#define REVERSE         1

void pid_init(double *input, double *output, double *setpoint,
        double kp, double ki, double kd, int controller_direction);
void pid_setMode(int mode);
uint8_t pid_compute(void);
void pid_setOutputLimits(double min, double max);
void pid_setTunings(double kp, double ki, double kd);
void pid_setCtrlDir(int dir);
void pid_setSampleTime(int ms);
double pid_getKp(void);
double pid_getKi(void);
double pid_getKd(void);
int pid_getMode(void);
int pid_getDir(void);

void initialize(void);

#endif
