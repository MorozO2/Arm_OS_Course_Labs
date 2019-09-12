/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)

#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "DigitalIoPin.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


SemaphoreHandle_t mut;

//Task for check sw1
static void sw1Task(void *pvParameters)
{
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, false);
	vTaskDelay(10);
	while(1)
	{
		if(sw1.read() == false){
			if(xSemaphoreTake(mut, portMAX_DELAY))
			{
				DEBUGOUT("SW1 pressed\r\n");
				xSemaphoreGive(mut);
			}
			while(sw1.read() == false){}

		}
	}
}
//Task for check sw2
static void sw2Task(void *pvParameters)
{
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, false);
	vTaskDelay(10);
	while(1)
	{
		if(sw2.read() == false){
			if(xSemaphoreTake(mut, portMAX_DELAY))
			{
				DEBUGOUT("SW2 pressed\r\n");
				xSemaphoreGive(mut);
			}
			while(sw2.read() == false){}

		}
	}
}


//Task for check sw3
static void sw3Task(void *pvParameters)
{
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, false);
	vTaskDelay(10);
	while(1)
	{
		if(sw3.read() == false){
			if(xSemaphoreTake(mut, portMAX_DELAY))
			{
				DEBUGOUT("SW3 pressed\r\n");
				xSemaphoreGive(mut);
			}
			while(sw3.read() == false){}

		}
	}
}



static void vUARTTask(void *pvParameters) {
	/*int secCnt_1 = 0;
	int secCnt_10 = 0;
	int minCnt_1 = 0;
	int minCnt_10 = 0;
	DigitalIoPin sw1(0, 17, DigitalIoPin::input, false);

	//Counting time
	while (1) {
		if(sw1.read() == false)
		{
			secCnt_10++;
		}
		else
		{
			secCnt_1++;
		}

		if(secCnt_1 > 9) {secCnt_1 = 0; secCnt_10++;}
		if(secCnt_10 > 5) {secCnt_10 = 0; minCnt_1++;}
		if(minCnt_1 > 9) {minCnt_1 = 0; minCnt_10++;}
		if(minCnt_10 > 5) {secCnt_1 = 0; secCnt_10 = 0; minCnt_1 = 0; minCnt_10 = 0;}
		DEBUGOUT("Time: %d%d:%d%d \r\n", minCnt_10, minCnt_1, secCnt_10, secCnt_1);
		/* About a 1s delay here */
		/*vTaskDelay(configTICK_RATE_HZ);
		}*/

}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{

	prvSetupHardware();
	mut = xSemaphoreCreateMutex();
	xTaskCreate(sw1Task, "sw1Task",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(sw2Task, "sw2Task",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(sw3Task, "sw3Task",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

