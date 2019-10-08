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
#include <stdio.h>
#include <time.h>

// TODO: insert other include files here
#include <string>
// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
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

EventGroupHandle_t event;
SemaphoreHandle_t mutx;
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();


}


static void Button_Task(void *pvParameters)
{

	DigitalIoPin sw1(0, 17, DigitalIoPin::input, true);
	vTaskDelay(10);
	while(1)
	{
		if(sw1.read())
		{
			xEventGroupSetBits(event, 1);
		}

	}
}

static void Task1(void *pvParameters)
{
	char task[] = "Task 1 ";
	xEventGroupWaitBits(event, 1, pdFALSE, pdFALSE, portMAX_DELAY);
	while(1)
	{



		srand(xTaskGetTickCount());
		vTaskDelay(rand() % 1000 + 1000);

		int ticks = xTaskGetTickCount();

		if(xSemaphoreTake(mutx, portMAX_DELAY))
		{
			DEBUGOUT("%s %d ms\r\n", task, ticks);
			xSemaphoreGive(mutx);
		}

	}
}
static void Task2(void *pvParameters)
{
	char task[] = "Task 2 ";
	xEventGroupWaitBits(event, 1, pdFALSE, pdFALSE, portMAX_DELAY);
	while(1)
	{


		srand(xTaskGetTickCount());
		vTaskDelay(rand() % 1000 + 1000);

		int ticks = xTaskGetTickCount();

		if(xSemaphoreTake(mutx, portMAX_DELAY))
		{
			DEBUGOUT("%s %d ms\r\n", task, ticks);
			xSemaphoreGive(mutx);
		}

	}
}
static void Task3(void *pvParameters)
{
	char task[] = "Task 3 ";
	xEventGroupWaitBits(event, 1, pdFALSE, pdFALSE, portMAX_DELAY);
	while(1)
	{


		srand(xTaskGetTickCount());
		vTaskDelay(rand() % 1000 + 1000);

		int ticks = xTaskGetTickCount();

		if(xSemaphoreTake(mutx, portMAX_DELAY))
		{
			DEBUGOUT("%s %d ms\r\n", task, ticks);
			xSemaphoreGive(mutx);
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
	mutx = xSemaphoreCreateMutex();
	event = xEventGroupCreate();


	xTaskCreate(Button_Task, "Button_Task",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task1, "Task1",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Task2",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task3, "Task3",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

