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
#include <stdlib.h>


// TODO: insert other include files here
#include <string>
// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"





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

QueueHandle_t prQ(xQueueCreate(6, sizeof(char[6])));
SemaphoreHandle_t bin(xSemaphoreCreateBinary());
TimerHandle_t oneShot;
TimerHandle_t reload;
static void oneShotCallBack(TimerHandle_t pxTimer);
static void reloadCallBack(TimerHandle_t pxTimer);

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}



static void Task1(void *pvParameters)
{
	char cmd[6];

	while(1)
	{
		if(xQueueReceive(prQ, &cmd, portMAX_DELAY))
		{
			DEBUGOUT("%s\r\n", cmd);
		}
	}
}
static void Task2(void *pvParameters)
{
	char argh[] = "argh";

	while(1)
	{
		if(xSemaphoreTake(bin, portMAX_DELAY))
		{
			xQueueSend(prQ, &argh, portMAX_DELAY);
		}
	}
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
	oneShot = xTimerCreate("oneShotCallback", 20000 / portTICK_PERIOD_MS, pdFALSE, (void*) 0, oneShotCallBack);
	reload =xTimerCreate("reloadCallBack", 5000 / portTICK_PERIOD_MS, pdTRUE, (void *)0, reloadCallBack);
	xTimerStart(oneShot, portMAX_DELAY);
	xTimerStart(reload, portMAX_DELAY);
	xTaskCreate(Task1, "Task1",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Task2",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

static void oneShotCallBack(TimerHandle_t pxTimer)
{
	xSemaphoreGive(bin);
}
static void reloadCallBack(TimerHandle_t pxTimer)
{
	char hello[] = "hello";

	xQueueSend(prQ, &hello, portMAX_DELAY);
}
