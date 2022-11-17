/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : YS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx.h"
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_TASK_EQ_0_ITERATION_NBR              16u
#define BSP_GPIOB_LED1	DEF_BIT_00
#define BSP_GPIOB_LED2	DEF_BIT_07
#define BSP_GPIOB_LED3	DEF_BIT_14
/*
*********************************************************************************************************
*                                            TYPES DEFINITIONS
*********************************************************************************************************
*/
typedef enum {
   TASK_500MS,
   TASK_1000MS,
   TASK_2000MS,

   TASK_N
}task_e;
typedef struct
{
   CPU_CHAR* name;
   OS_TASK_PTR func;
   OS_PRIO prio;
   CPU_STK* pStack;
   OS_TCB* pTcb;
}task_t;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void	  *p_arg);
static  void  AppObjCreate          (void);

static void AppTask_led1blink(void *p_arg);
static void AppTask_led2blink(void *p_arg);
static void AppTask_led3blink(void *p_arg);

/***********************************/
/**********declared for HW2*********/
// led on/off functions
static void led_on();
static void led_off();

// setting global variables for configure blink task
// blinking terms of each led units
static int led1_blink_term = 1;
static int led2_blink_term = 1;
static int led3_blink_term = 1;
// 0 : not blinking, 1 : blinking
static int led1_blink = 1;
static int led2_blink = 1;
static int led3_blink = 1;
// blinking configure functions
static void configure_led1_blink(int tf); // parameter tf(true false) : 1: true, 2 : false
static void configure_led2_blink(int tf);
static void configure_led3_blink(int tf);
// blinking term configure functions
static void configure_led1_blink_term(int bt); // parameter bt(blink time) -> set to blinking term variavle
static void configure_led2_blink_term(int bt);
static void configure_led3_blink_term(int bt);
/***********************************/

static void Setup_Gpio(void);


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       AppTask_led1blink_TCB;
static  OS_TCB       AppTask_led2blink_TCB;
static  OS_TCB       AppTask_led3blink_TCB;

static  CPU_STK  AppTask_led1blink_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  AppTask_led2blink_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  AppTask_led3blink_Stack[APP_CFG_TASK_START_STK_SIZE];
int count=0;
task_t cyclic_tasks[TASK_N] = {
   {"AppTask_led1blink", AppTask_led1blink, 0, &AppTask_led1blink_Stack[0], &AppTask_led1blink_TCB},
   {"AppTask_led2blink", AppTask_led2blink, 0, &AppTask_led2blink_Stack[0], &AppTask_led2blink_TCB},
   {"AppTask_led3blink", AppTask_led3blink, 0, &AppTask_led3blink_Stack[0], &AppTask_led3blink_TCB},
};
/* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
    OS_ERR  err;

    /* Basic Init */
    RCC_DeInit();
//    SystemCoreClockUpdate();
    Setup_Gpio();

    /* BSP Init */
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */


    /* OS Init */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

   OSStart(&err);   /* Start multitasking (i.e. give control to uC/OS-III). */

   (void)&err;

   return (0u);
}
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
    OS_ERR  err;

   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    BSP_Tick_Init();                                            /* Initialize Tick Services.                            */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

   // BSP_LED_Off(0u);                                            /* Turn Off LEDs after initialization                   */

   APP_TRACE_DBG(("Creating Application Kernel Objects\n\r"));
   AppObjCreate();                                             /* Create Applicaiton kernel objects                    */

   // APP_TRACE_DBG(("Creating Application Tasks\n\r"));                                        /* Create Application tasks                             */
   APP_TRACE_DBG(("LED ON/OFF test\n\r"));
   led_on(1);
   led_on(2);
   led_on(3);
   AppTaskCreate(p_arg);
}

/*
*********************************************************************************************************
*                                          AppTask_500ms
*
* Description : Example of 500mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_led1blink(void *p_arg)
{
	/*ledparam : XY => X : for led number, Y : for time unit*/
    OS_ERR  err;
    BSP_LED_On(1);

    if(led1_blink)
    {
        while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
            BSP_LED_Toggle(1);

            OSTimeDlyHMSM(0u, 0u, led1_blink_term, 0u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);

        }
    }
}

/*
*********************************************************************************************************
*                                          AppTask_1000ms
*
* Description : Example of 1000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_led2blink(void *p_arg)
{
	/*ledparam : XY => X : for led number, Y : for time unit*/
    OS_ERR  err;
    BSP_LED_On(2);

    if(led2_blink)
    {
        while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
            BSP_LED_Toggle(2);

            OSTimeDlyHMSM(0u, 0u, led2_blink_term, 0u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);

        }
    }
}

/*
*********************************************************************************************************
*                                          AppTask_2000ms
*
* Description : Example of 2000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_led3blink(void *p_arg)
{
	/*ledparam : XY => X : for led number, Y : for time unit*/
    OS_ERR  err;
    BSP_LED_On(3);

    if(led3_blink)
    {
        while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
            BSP_LED_Toggle(3);

            OSTimeDlyHMSM(0u, 0u, led3_blink_term, 0u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);

        }
    }

}
/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void *p_arg)
{
   OS_ERR  err;
   int idx;
   task_t* pTask_Cfg;
   for(idx = 0; idx < TASK_N; idx++)
   {
	   pTask_Cfg = &cyclic_tasks[idx]; // don't change... blink task was implemented by 1000ms unit

	   OSTaskCreate(
			 pTask_Cfg->pTcb,
			 pTask_Cfg->name,
			 pTask_Cfg->func,
			 p_arg,
			 pTask_Cfg->prio,
			 pTask_Cfg->pStack,
			 pTask_Cfg->pStack[APP_CFG_TASK_START_STK_SIZE / 10u],
			 APP_CFG_TASK_START_STK_SIZE,
			 (OS_MSG_QTY    )0u,
			 (OS_TICK       )0u,
			 (void         *)0u,
			 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			 (OS_ERR       *)&err
	   );
   }
}


/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppObjCreate (void)
{

}

/*
*********************************************************************************************************
*                                          Setup_Gpio()
*
* Description : Configure LED GPIOs directly
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     :
*              LED1 PB0
*              LED2 PB7
*              LED3 PB14
*
*********************************************************************************************************
*/
static void Setup_Gpio(void)
{
   GPIO_InitTypeDef led_init = {0};
   GPIO_InitTypeDef USART_init = {0};

   /* setup GPIO for LED */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   /* setup LED Alternative Function */
   led_init.GPIO_Mode   = GPIO_Mode_OUT;
   led_init.GPIO_OType  = GPIO_OType_PP;
   led_init.GPIO_Speed  = GPIO_Speed_2MHz;
   led_init.GPIO_PuPd   = GPIO_PuPd_NOPULL;
   led_init.GPIO_Pin    = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;
   GPIO_Init(GPIOB, &led_init);


   /* setup GPIO for USART3 */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // What is difference between USARTx blocks???
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // PD8 : Tx, PD9 : Rx
   /* setup Rx, Tx Alternative Function */
   GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
   GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
   USART_init.GPIO_Mode = GPIO_Mode_AF;
   USART_init.GPIO_OType = GPIO_OType_PP;
   USART_init.GPIO_Speed = GPIO_Speed_2MHz;
   USART_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   USART_init.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
   GPIO_Init(GPIOD, &USART_init);

}

static void led_on(int led_num)
{
	switch(led_num)
	{
	case 1:
		GPIO_SetBits(  GPIOB, BSP_GPIOB_LED1);
		break;
	case 2:
		GPIO_SetBits(  GPIOB, BSP_GPIOB_LED2);
		break;
	case 3:
		GPIO_SetBits(  GPIOB, BSP_GPIOB_LED3);
		break;
	default:
		break;
	}

}

static void led_off(int led_num)
{
	switch(led_num)
	{
	case 1:
		GPIO_ResetBits(  GPIOB, BSP_GPIOB_LED1);
		break;
	case 2:
		GPIO_ResetBits(  GPIOB, BSP_GPIOB_LED2);
		break;
	case 3:
		GPIO_ResetBits(  GPIOB, BSP_GPIOB_LED3);
		break;
	default:
		break;
	}

}

static void configure_led1_blink(int tf)
{
	if(tf)
	{
		led1_blink = 1;
	}
	else led1_blink = 0;
}

static void configure_led2_blink(int tf)
{
	if(tf)
	{
		led2_blink = 1;
	}
	else led2_blink = 0;
}

static void configure_led3_blink(int tf)
{
	if(tf)
	{
		led3_blink = 1;
	}
	else led3_blink = 0;
}

static void configure_led1_blink_term(int bt)
{
	if(led1_blink)
	{
		led1_blink_term = bt;
	}
}
static void configure_led2_blink_term(int bt)
{
	if(led2_blink)
	{
		led2_blink_term = bt;
	}
}
static void configure_led3_blink_term(int bt)
{
	if(led3_blink)
	{
		led3_blink_term = bt;
	}
}