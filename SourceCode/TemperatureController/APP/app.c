#define GLOBALS

#include "stdarg.h"

#include "includes.h"
#include "globals.h"
#include "app.h"
#include "ds18b20.h"
#include "fan.h"
#include "ads1100.h"
#include "bluetooth.h"
#include "m25p16.h"
#include "lcd1602.h"
#include "math.h"
#include "pid.h"

#define SSR_PERIOD      2000
#define SSR_PERIOD_PER  20
#define SSR_MAX         100

#define SAMPLE_SIZE        11
#define DEFAULT_OUTPUT      30

OS_EVENT* atCmdMailbox;
int isRunning = 0;
int time;
int temperature;

double mTemp0[SAMPLE_SIZE];
double mTemp1[SAMPLE_SIZE];

int s_count = 0;
int s_speed[10];
int s_temperature[10];
int s_time[10];
double s_ssr1 = 0;
double s_ssr2 = 0;
int s_fan1 = 0;
int s_fan2 = 0;
/*
 *********************************************************************************************************
 *                                            LOCAL DEFINES
 *********************************************************************************************************
 */



/*
 *********************************************************************************************************
 *                                       LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 */

static OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
//process at command
static OS_STK task_process_atcmd_stk[task_process_atcmd_stk_size];
//report work
__align(8) static OS_STK task_report_work_stk[task_report_work_stk_size];
//set ssr process
__align(8) static OS_STK task_set_ssr1_stk[task_set_ssr1_size];
static OS_STK task_set_ssr2_stk[task_set_ssr2_size];

/*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
static  void App_TaskCreate(void);
static  void App_TaskStart(void* p_arg);
//process atcmd task
static void task_process_atcmd(void *parg);
//work process
static void task_report_work(void *parg);
//set ssr
static void task_set_ssr1(void *parg);
static void task_set_ssr2(void *parg);

int main(void)
{
    CPU_INT08U os_err;
    //禁止CPU中断
    CPU_IntDis();
    //UCOS 初始化
    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */
    //硬件平台初始化
    BSP_Init();                                                 /* Initialize BSP functions.  */
    printk("Hello, This is Temperature Controller System!\r\n");
    os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,	  		  		//指向任务代码的指针
            (void *) 0,								  		//任务开始执行时，传递给任务的参数的指针
            (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],	//分配给任务的堆栈的栈顶指针   从顶向下递减
            (INT8U) APP_TASK_START_PRIO);								//分配给任务的优先级

    if(os_err != OS_ERR_NONE) {
        printk("create main task failed\r\n");
    }
    OSTimeSet(0);
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */

    return (0);
}

/*
 *********************************************************************************************************
 *                                          App_TaskStart()
 *
 * Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
 *
 * Argument : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
 *
 * Return   : none.
 *
 * Caller   : This is a task.
 *
 * Note     : none.
 *********************************************************************************************************
 */

static  void App_TaskStart(void* p_arg)
{
    (void) p_arg;

    //初始化ucos时钟节拍
    OS_CPU_SysTickInit();                                       /* Initialize the SysTick.       */

    //使能ucos 的统计任务
#if (OS_TASK_STAT_EN > 0)
    //----统计任务初始化函数  
    OSStatInit();                                               /* Determine CPU capacity.                              */
#endif
    //建立其他的任务

    App_TaskCreate();

    while (1) {
        OSTimeDlyHMSM(0, 0, 5, 0);
    }
}

/*
 *********************************************************************************************************
 *                                            App_TaskCreate()
 *
 * Description : Create the application tasks.
 *
 * Argument : none.
 *
 * Return   : none.
 *
 * Caller   : App_TaskStart().
 *
 * Note     : none.
 *********************************************************************************************************
 */

static  void App_TaskCreate(void)
{
    OSTaskCreateExt(task_process_atcmd,
            (void *)0,
            (OS_STK *)&task_process_atcmd_stk[task_process_atcmd_stk_size - 1],
            task_process_atcmd_prio,
            task_process_atcmd_prio,
            (OS_STK *)&task_process_atcmd_stk[0],
            task_process_atcmd_stk_size,
            (void *)0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(task_report_work,
            (void *)0,
            (OS_STK *)&task_report_work_stk[task_report_work_stk_size - 1],
            task_report_work_prio,
            task_report_work_prio,
            (OS_STK *)&task_report_work_stk[0],
            task_report_work_stk_size,
            (void *)0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskCreateExt(task_set_ssr1,
            (void *)0,
            (OS_STK *)&task_set_ssr1_stk[task_set_ssr1_size - 1],
            task_set_ssr1_prio,
            task_set_ssr1_prio,
            (OS_STK *)&task_set_ssr1_stk[0],
            task_set_ssr1_size,
            (void *)0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskCreateExt(task_set_ssr2,
            (void *)0,
            (OS_STK *)&task_set_ssr2_stk[task_set_ssr2_size - 1],
            task_set_ssr2_prio,
            task_set_ssr2_prio,
            (OS_STK *)&task_set_ssr2_stk[0],
            task_set_ssr2_size,
            (void *)0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}

static double convert_temp(double input);

double current_temp = 0.0f;

double get_ratio(int interval)
{
    uint8_t i = 0;
    double ratio = 0;
    uint8_t valid_size = SAMPLE_SIZE;;

    for(i = 0; i < SAMPLE_SIZE; i++) {
        if(mTemp0[i] == 0.0f){
            valid_size = i;
            break;
        }
    }

    if(valid_size > 0) {
        ratio = (mTemp0[valid_size - 1] - mTemp0[0]) / interval;
    }

    for(i = 0; i < SAMPLE_SIZE; i++) {
        printf("%.4f  ", mTemp0[i]);
    }
    printf("\r\n");

    return ratio;
}

//main task
static void task_process_atcmd(void *parg)
{
    uint16_t result0, result1;
    uint8_t i = 0;
    int delta[SAMPLE_SIZE - 1];
    double ratio;
    double point = 10.0f / 60.0f;
    static double output;
    double D = 60.0f;

    (void)parg;

    twi_init();

    set_ssr1(DEFAULT_OUTPUT);
    while(1) {
        result0 = ads1100_get_result(0);
        OSTimeDlyHMSM(0, 0, 0, 10);
        result1 = ads1100_get_result(1);
        OSTimeDlyHMSM(0, 0, 0, 10);

        mTemp0[10] = mTemp0[9];
        mTemp0[9] = mTemp0[8];
        mTemp0[8] = mTemp0[7];
        mTemp0[7] = mTemp0[6];
        mTemp0[6] = mTemp0[5];
        mTemp0[5] = mTemp0[4];
        mTemp0[4] = mTemp0[3];
        mTemp0[3] = mTemp0[2];
        mTemp0[2] = mTemp0[1];
        mTemp0[1] = mTemp0[0];

        mTemp0[0] = convert_temp(result0);
        mTemp1[0] = convert_temp(result1);

        current_temp = mTemp0[0];
        ratio = get_ratio(1);
        printf("ratio = %f\r\n", ratio);
        output = output - D * (ratio - point);

        printf("output = %f\r\n", output);
        if(output > SSR_MAX) {
            output = SSR_MAX;
        } else if(output < SSR_MIN) {
            output = SSR_MIN;
        }
        if(mTemp0[10] != 0) {
            set_ssr1(output);
            printf("output = %f\r\n", output);
        }

        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

static double convert_temp(double input)
{
    double Rt;
    double x;
    double A = 3.9083;
    double B = -5.775;
    double c;
    double T;
    double v;

    v = 0.5 + input / 32768.0;
    Rt = v * 500.0 / (1.0 - v);
    x = 1.0 - Rt / 1000.0;
    c = 10000.0 * sqrt(A * A - 0.4 * B * x);
    T = (c - A * 10000.0) / 2.0 / B;

    return T;
}

static void task_report_work(void *parg)
{
    (void)parg;

    OSTimeDlyHMSM(0, 0, 2, 0);
    while(1) {
        report_temperature(time++, (int)(current_temp * 100),
                (int)(current_temp * 100));
        printf("T0: %f\r\n", current_temp);
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

static uint8_t isStop = TRUE;

static void set_ssr1(double ssr)
{
    if(isStop) {
        s_ssr1 = 0;
    } else {
        s_ssr1 = ssr;
    }
}

static void set_ssr2(double ssr)
{
    if(isStop) {
        s_ssr2 = 0;
    } else {
        s_ssr2 = ssr;
    }
}

static void task_set_ssr1(void *parg)
{
    (void)parg;

    while(1) {
        //printk("ssr1 enable\r\n");
        SSR1_Enable(1);
        OSTimeDlyHMSM(0, 0, 0, SSR_PERIOD_PER * s_ssr1);
        //printk("ssr1 disable\r\n");
        SSR1_Enable(0);
        OSTimeDlyHMSM(0, 0, 0, SSR_PERIOD_PER * (SSR_MAX - s_ssr1));
    }
}

static void task_set_ssr2(void *parg)
{
    (void)parg;

    while(1) {
        //printk("ssr2 enable\r\n");
        SSR2_Enable(1);
        OSTimeDlyHMSM(0, 0, 0, SSR_PERIOD_PER * s_ssr2);
        //printk("ssr2 disable\r\n");
        SSR2_Enable(0);
        OSTimeDlyHMSM(0, 0, 0, SSR_PERIOD_PER * (SSR_MAX - s_ssr2));
    }
}
/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                          uC/OS-II APP HOOKS
 *********************************************************************************************************
 *********************************************************************************************************
 */

#if (OS_APP_HOOKS_EN > 0)
/*
 *********************************************************************************************************
 *                                      TASK CREATION HOOK (APPLICATION)
 *
 * Description : This function is called when a task is created.
 *
 * Argument : ptcb   is a pointer to the task control block of the task being created.
 *
 * Note     : (1) Interrupts are disabled during this call.
 *********************************************************************************************************
 */

void App_TaskCreateHook(OS_TCB* ptcb)
{
}

/*
 *********************************************************************************************************
 *                                    TASK DELETION HOOK (APPLICATION)
 *
 * Description : This function is called when a task is deleted.
 *
 * Argument : ptcb   is a pointer to the task control block of the task being deleted.
 *
 * Note     : (1) Interrupts are disabled during this call.
 *********************************************************************************************************
 */

void App_TaskDelHook(OS_TCB* ptcb)
{
    (void) ptcb;
}

/*
 *********************************************************************************************************
 *                                      IDLE TASK HOOK (APPLICATION)
 *
 * Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
 *               has been added to allow you to do such things as STOP the CPU to conserve power.
 *
 * Argument : none.
 *
 * Note     : (1) Interrupts are enabled during this call.
 *********************************************************************************************************
 */

#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

/*
 *********************************************************************************************************
 *                                        STATISTIC TASK HOOK (APPLICATION)
 *
 * Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
 *               statistics task.  This allows your application to add functionality to the statistics task.
 *
 * Argument : none.
 *********************************************************************************************************
 */

void App_TaskStatHook(void)
{
}

/*
 *********************************************************************************************************
 *                                        TASK SWITCH HOOK (APPLICATION)
 *
 * Description : This function is called when a task switch is performed.  This allows you to perform other
 *               operations during a context switch.
 *
 * Argument : none.
 *
 * Note     : 1 Interrupts are disabled during this call.
 *
 *            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
 *                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
 *                  task being switched out (i.e. the preempted task).
 *********************************************************************************************************
 */

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

/*
 *********************************************************************************************************
 *                                     OS_TCBInit() HOOK (APPLICATION)
 *
 * Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
 *               up most of the TCB.
 *
 * Argument : ptcb    is a pointer to the TCB of the task being created.
 *
 * Note     : (1) Interrupts may or may not be ENABLED during this call.
 *********************************************************************************************************
 */

#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
    (void) ptcb;
}
#endif

#endif
