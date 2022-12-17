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
#include "bsp.h"
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
static void AppTask_Countinit(void *p_arg);

void DHT11_Start(void);
uint8_t DHT11_response(void);
uint8_t DHT11_read(void);
void send_intval(unsigned char data);
static void RGB_RED();
static void RGB_BLUE();
static void RGB_GREEN();
static void MotorSpeed(uint32_t speed);
static void ServoControl(uint32_t angle);
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

static  OS_TCB   Task_CountInit_TCB;
static  CPU_STK  Task_CountInit_Stack[APP_CFG_TASK_START_STK_SIZE];

static  OS_Q	 BUT_Q;

static int click = 0;
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
static int state = 0;
static int lednum = 0;
static void AppTask_1000ms(void *p_arg)
{
    OS_ERR  err;
    BSP_LED_Off(1);
    BSP_LED_Off(2);
    BSP_LED_Off(3);

    while (DEF_TRUE)
    {                                          /* Task body, always written as an infinite loop.       */

		BSP_LED_Toggle(1);
		BSP_LED_Toggle(2);
		BSP_LED_Toggle(3);

        OSTimeDlyHMSM(0u, 0u, 1u, 0u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

    }
}

static void AppTask_ButtonInput(void *p_arg)
{
    OS_ERR  err;
    int button = 0;
    int prev_button = 0;
    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */


    	button = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);

    	if(button != prev_button)
    	{
        	click++;
        	prev_button = button;
    	}


        OSQPost( (OS_Q *)&BUT_Q,
        		 (void *)click,
    			 (OS_MSG_SIZE)sizeof(int *),
    			 (OS_OPT)OS_OPT_POST_ALL,
    			 (OS_ERR *)&err);

        OSTimeDlyHMSM(0u, 0u, 0u, 250u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);

    }
}

static int startflag = 1;
static void AppTask_USART(void *p_arg)
{
	OS_ERR err;
	uint8_t Rh_byte1 = 0;
	uint8_t Rh_byte2 = 0;
	uint8_t Temp_byte1 = 0;
	uint8_t Temp_byte2 = 0;
	uint8_t checksum = 0;

	while (DEF_TRUE)
	{
		if(startflag)
		{
			send_string("\n\rSystem start \n\r");
			startflag = 0;
		}

		RGB_BLUE();
		MotorSpeed(9999);
		ServoControl(499999);

		OSTimeDlyHMSM(0u, 0u, 1u, 0u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
	}
}

static void AppTask_CountInit(void *p_arg)
{
	OS_ERR err;


	// Initialize click count every 4seconds
	while (DEF_TRUE)
	{
		click = 0;

		OSTimeDlyHMSM(0u, 0u, 2u, 0u,
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
		(OS_PRIO       )8, // set priority
		(CPU_STK      *)&Task_1000ms_Stack[0u],
		(CPU_STK_SIZE  )Task_1000ms_Stack[APP_CFG_TASK_START_STK_SIZE / 10u],
		(CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
		(OS_MSG_QTY    )0u,
		(OS_TICK       )0u,
		(void         *)0u,
		(OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		(OS_ERR       *)&err);

	// USART Task, priority 7, term : 2 second
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

void TIM_Configure(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// Timer 2 : count 18ms
	TIM_TimeBaseStructure.TIM_Period = 18000; // 18000us == 18ms
	TIM_TimeBaseStructure.TIM_Prescaler = 45-1; // count every 1us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	// Timer 3 : count 40us
	TIM_TimeBaseStructure.TIM_Period = 40; // 40us
	TIM_TimeBaseStructure.TIM_Prescaler = 45-1; // count every 1us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/*
	// Timer 4 : count 80us
	TIM_TimeBaseStructure.TIM_Period = 80; // 80us
	TIM_TimeBaseStructure.TIM_Prescaler = 45-1; // count every 1us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	 */
}

static  void  AppObjCreate (void)
{
	OS_ERR err;
	TIM_Configure();

	OSQCreate((OS_Q *)&BUT_Q,
			  (CPU_CHAR *)"Button Queue",
			  (OS_MSG_QTY)1, // Queue size = 1
			  (OS_ERR *)&err);
}

/*
	DC Motor Duty Cycle
	0%   : 4,500 *   (0/100) - 1 = 0
	25%  : 4,500 *  (25/100) - 1 = 1,124
	50%  : 4,500 *  (50/100) - 1 = 2,249
	75%  : 4,500 *  (75/100) - 1 = 3,374
	100% : 4,500 * (100/100) - 1 = 4,999
*/
static void MotorSpeed(uint32_t speed)
{
	TIM_OCInitTypeDef TIM_OCStruct;
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OCStruct.TIM_Pulse = speed;
	TIM_OC1Init(TIM3, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

/*
	RGB LED Duty Cycle
	0%   : 450,000 *   (0/100) - 1 = 0
	25%  : 450,000 *  (25/100) - 1 = 124,999
	50%  : 450,000 *  (50/100) - 1 = 249,999
	75%  : 450,000 *  (75/100) - 1 = 374,999
	100% : 450,000 * (100/100) - 1 = 499,999
*/
static void RGB_RED()
{
	TIM_OCInitTypeDef TIM_OCStruct;
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	/* red use PWM channel 1 */
	TIM_OCStruct.TIM_Pulse = 499999;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* make other color's PWM to 0 */
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC2Init(TIM4, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC3Init(TIM4, &TIM_OCStruct);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

static void RGB_BLUE()
{
	TIM_OCInitTypeDef TIM_OCStruct;
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	/* blue use PWM channel 2 */
	TIM_OCStruct.TIM_Pulse = 499999;
	TIM_OC2Init(TIM4, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* make other color's PWM to 0 */
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC3Init(TIM4, &TIM_OCStruct);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

static void RGB_GREEN()
{
	TIM_OCInitTypeDef TIM_OCStruct;
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	/* green use PWM channel 3 */
	TIM_OCStruct.TIM_Pulse = 499999;
	TIM_OC3Init(TIM4, &TIM_OCStruct);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	/* make other color's PWM to 0 */
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC2Init(TIM4, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

static void ServoControl(uint32_t angle)
{
	TIM_OCInitTypeDef TIM_OCStruct;
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	/* green use PWM channel 3 */
	TIM_OCStruct.TIM_Pulse = angle;
	TIM_OC4Init(TIM4, &TIM_OCStruct);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
}


/*
// wait for 18ms
void delay18ms(void)
{
	TIM_Cmd(TIM2,ENABLE);
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);

	while(TIM_GetITStatus(TIM2, TIM_IT_Update) == RESET);
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	TIM_Cmd(TIM2,DISABLE);
}

// wait for 40us
void delay40us(void)
{
	TIM_Cmd(TIM3,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE);

	while(TIM_GetITStatus(TIM3, TIM_IT_Update) == RESET);
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	TIM_Cmd(TIM3,DISABLE);
}

// wait for 80us
void delay80us(void)
{
	TIM_Cmd(TIM4,ENABLE);
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);

	while(TIM_GetITStatus(TIM4, TIM_IT_Update) == RESET);
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

	TIM_Cmd(TIM4,DISABLE);
}

void DHT11_Start(void)
{
	// set pin output low
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);

    // wait for 18ms
    delay18ms();
    // set pin output high
    GPIO_SetBits(GPIOC, GPIO_Pin_8);
    GPIO_SetBits(GPIOC, GPIO_Pin_9);

    // set pin as input
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.GPIO_Mode		= GPIO_Mode_IN;
    gpio_init.GPIO_OType	= GPIO_OType_PP;
    gpio_init.GPIO_Speed	= GPIO_Speed_2MHz;
    gpio_init.GPIO_PuPd		= GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Pin		= GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOC, GPIO_Pin_8);
    GPIO_Init(GPIOC, GPIO_Pin_8);
}

uint8_t DHT11_response(void)
{
	uint8_t response = 0;

	delay40us();
	if(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8))
	{
		delay80us();
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)) response = 1;
		else
		{
			response = -1;
			send_string("\n\rresponse is -1 \n\r");
		}
	}
	while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)); // wait until value is 0

	return response;
}

uint8_t DHT11_read(void)
{
	send_string("\n\rentering DHT11 Read function \n\r");
	uint8_t i, j;
	send_string("\n\rbefore for loop \n\r");
	for(j = 0; j < 8; j++)
	{
		send_string("\n\rbefore while loop \n\r");
		while(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)); // ��� 0�̿��� ���� Ż���� �ȵ�
		send_string("\n\rafter for loop \n\r");
		delay40us();
		if(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8))
		{
			i &= ~(1<<(7-j));
		}
		else i |= (1<<(7-j));
		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)); // wait until value go to 0
	}
	return i;
}

void send_intval(unsigned char data)
{
    while (USART_GetFlagStatus(Nucleo_COM1, USART_FLAG_TXE) == RESET);
    USART_SendData(Nucleo_COM1, data);
}*/
