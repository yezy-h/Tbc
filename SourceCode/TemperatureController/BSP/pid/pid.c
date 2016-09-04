#include "pid.h"
#include "stm32f10x.h"

double mDispKp;
double mDispKi;
double mDispKd;

double mKp;
double mKi;
double mKd;
int mCtrlDir;
double *mInput;
double *mOutput;
double *mSetpoint;

unsigned long lastTime;
double ITerm, lastInput;

unsigned long sampleTime;
double outMin, outMax;
uint8_t inAuto;

void pid_init(double *input, double *output, double *setpoint,
        double kp, double ki, double kd, int controller_direction)
{
    mOutput = output;
    mInput = input;
    mSetpoint = setpoint;
    inAuto = FALSE;

    pid_setOutputLimits(0, 65535);
    sampleTime = 1000;
    pid_setCtrlDir(controller_direction);
    pid_setTunings(kp, ki, kd);

    lastTime = CURRENT_TIME - sampleTime;
}

void pid_setMode(int mode)
{
    uint8_t newAuto = (mode == AUTOMATIC);
    if(newAuto == !inAuto) {
        initialize();
    }
    inAuto = newAuto;
}


uint8_t pid_compute(void)
{
    unsigned long now = CURRENT_TIME;
    unsigned long timeChange = (now - lastTime);
    double input;
    double error;
    double dInput;
    double output;

    //printf("now = %ld\r\n", now);
    if(!inAuto)
        return FALSE;
    if(timeChange >= sampleTime) {
        input = *mInput;
        error = *mSetpoint - input;
        ITerm += (mKi * error);
        if(ITerm > outMax)
            ITerm = outMax;
        else if(ITerm < outMin)
            ITerm = outMin;

        dInput = input - lastInput;

        //compute pid output
        output = mKp * error + ITerm - mKd * dInput;

        if(output > outMax)
            output = outMax;
        else if(output < outMin)
            output = outMin;

        *mOutput = output;

        lastInput = input;
        lastTime = now;
        return TRUE;
    } else {
        return FALSE;
    }
}

void pid_setOutputLimits(double min, double max)
{
    if(min >= max)
        return;
    outMin = min;
    outMax = max;

    if(inAuto) {
        if(*mOutput > outMax)
            *mOutput = outMax;
        else if(*mOutput < outMin)
            *mOutput = outMin;

        if(ITerm > outMax)
            ITerm = outMax;
        else if(ITerm < outMin)
            ITerm = outMin;
    }
}

void pid_setTunings(double kp, double ki, double kd)
{
    double sampleTimeInSec = ((double)sampleTime) / 1000.0f;
    if(kp < 0 || ki < 0 || kd < 0)
        return;

    mDispKd = kd;
    mDispKi = ki;
    mDispKp = kp;

    mKp = kp;
    mKi = ki * sampleTimeInSec;
    mKd = kd / sampleTimeInSec;

    if(mCtrlDir == REVERSE) {
        mKp = 0 - mKp;
        mKi = 0 - mKi;
        mKd = 0 - mKd;
    }
}

void pid_setCtrlDir(int dir)
{
    if(inAuto && dir != mCtrlDir) {
        mKp = 0 - mKp;
        mKi = 0 - mKi;
        mKd = 0 - mKd;
    }

    mCtrlDir = dir;
}

void pid_setSampleTime(int ms)
{
    double ratio;

    if(ms > 0) {
        ratio = (double)ms / (double)sampleTime;
        mKi *= ratio;
        mKd /= ratio;

        sampleTime = (unsigned long)ms;
    }
}

double pid_getKp(void)
{
    return mDispKp;
}

double pid_getKi(void)
{
    return mDispKi;
}

double pid_getKd(void)
{
    return mDispKd;
}

int pid_getMode(void)
{
    return inAuto ? AUTOMATIC : MANUAL;
}
int pid_getDir(void)
{
    return mCtrlDir;
}

void initialize(void)
{
    ITerm = *mOutput;
    lastInput = *mInput;
    if(ITerm > outMax)
        ITerm = outMax;
    else if(ITerm < outMin)
        ITerm = outMin;
}
