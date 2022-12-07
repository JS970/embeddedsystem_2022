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
/*
*********************************************************************************************************
*                                            TYPES DEFINITIONS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);

static void AppTask_1000ms(void *p_arg);
static void AppTask_ButtonInput(void *p_arg);
static void AppTask_USART(void *p_arg);

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   Task_1000ms_TCB;
static  CPU_STK  Task_1000ms_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   Task_ButtonInput_TCB;
static  CPU_STK  Task_ButtonInput_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   Task_USART_TCB;
static  CPU_STK  Task_USART_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_Q	 BUT_Q;
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
    CPU_Init();

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

   APP_TRACE_DBG(("Creating Application Tasks\n\r"));
   AppTaskCreate();                                            /* Create Application tasks                             */
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
static void AppTask_1000ms(void *p_arg)
{
    OS_ERR  err;
    BSP_LED_On(2);
    int message;
    int lednum = 0;

    while (DEF_TRUE)
    {                                          /* Task body, always written as an infinite loop.       */

		message = (int)OSQPend((OS_Q *)&BUT_Q,
							   (OS_TICK)0u, // wait forever
							   (OS_OPT)OS_OPT_PEND_BLOCKING,
							   (OS_MSG_SIZE *)sizeof(int *),
							   (CPU_TS *)0,
							   (OS_ERR *)&err);

    	if(message == 0)
    	{
        	BSP_LED_Off(2);
        	BSP_LED_Off(3);
        	BSP_LED_Toggle(1);
    	}

    	if(message == 1)
    	{
    		lednum++;
    		lednum %= 3;
        	BSP_LED_Off((lednum+1) % 3 + 1);
        	BSP_LED_Off((lednum+2) % 3 + 1);
        	BSP_LED_Toggle(lednum+1);
    	}

    	if(message >= 2)
    	{
    		BSP_LED_Off(1);
    		BSP_LED_Off(2);
    		BSP_LED_Off(3); // WHY?????
    		BSP_LED_Toggle(1);
    		BSP_LED_Toggle(2);
        	BSP_LED_Toggle(3);
    	}

    	OSTimeDlyHMSM(0u, 0u, 1u, 0u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

    }
}

static void AppTask_ButtonInput(void *p_arg)
{
    OS_ERR  err;
    int button = 0;
    int click = 0;

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */


    	button = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);

    	if(button)
    	{
        	click++;
    	}

        OSQPost( (OS_Q *)&BUT_Q,
        		 (void *)click,
    			 (OS_MSG_SIZE)sizeof(int *),
    			 (OS_OPT)OS_OPT_POST_ALL,
    			 (OS_ERR *)&err);

    	OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

    }
}

static void AppTask_USART(void *p_arg)
{
	OS_ERR err;
	int message;

	while (DEF_TRUE) {

		message = (int)OSQPend((OS_Q *)&BUT_Q,
							   (OS_TICK)0u, // wait forever
							   (OS_OPT)OS_OPT_PEND_NON_BLOCKING,
							   (OS_MSG_SIZE *)sizeof(int *),
							   (CPU_TS *)0,
							   (OS_ERR *)&err);

		OSTimeDlyHMSM(0u, 0u, 0u, 500u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
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

static  void  AppTaskCreate (void)
{
	OS_ERR  err;

	OSTaskCreate(
		(OS_TCB       *)&Task_1000ms_TCB,              /*       LED task        */
		(CPU_CHAR     *)"AppTask_1000ms",
		(OS_TASK_PTR   )AppTask_1000ms,
		(void         *)0u,
		(OS_PRIO       )5, // set priority
		(CPU_STK      *)&Task_1000ms_Stack[0u],
		(CPU_STK_SIZE  )Task_1000ms_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY    )0u,
		(OS_TICK       )0u,
		(void         *)0u,
		(OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR       *)&err);

	OSTaskCreate(
		(OS_TCB       *)&Task_ButtonInput_TCB,         /*    ButtonInput task    */
		(CPU_CHAR     *)"AppTask_ButtonInput",
		(OS_TASK_PTR   )AppTask_ButtonInput,
		(void         *)0u,
		(OS_PRIO       )6, // set priority
		(CPU_STK      *)&Task_ButtonInput_Stack[0u],
		(CPU_STK_SIZE  )Task_ButtonInput_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY    )0u,
		(OS_TICK       )0u,
		(void         *)0u,
		(OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR       *)&err);

	OSTaskCreate(
		(OS_TCB       *)&Task_USART_TCB,               /*      USART task         */
		(CPU_CHAR     *)"AppTask_USART",
		(OS_TASK_PTR   )AppTask_USART,
		(void         *)0u,
		(OS_PRIO       )7, // set priority
		(CPU_STK      *)&Task_USART_Stack[0u],
		(CPU_STK_SIZE  )Task_USART_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY    )0u,
		(OS_TICK       )0u,
		(void         *)0u,
		(OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR       *)&err);

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
	OS_ERR err;

	OSQCreate((OS_Q *)&BUT_Q,
			  (CPU_CHAR *)"Button Queue",
			  (OS_MSG_QTY)1, // Queue size = 1
			  (OS_ERR *)&err);
}