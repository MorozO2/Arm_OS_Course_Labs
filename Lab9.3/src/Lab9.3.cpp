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
#include "DigitalIoPin.h"

#define BT_1 (1 << 0)
#define BT_2 (1 << 1)
#define BT_3 (1 << 2)
#define BITS (BT_1 | BT_2 | BT_3)


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

EventGroupHandle_t event(xEventGroupCreate());
SemaphoreHandle_t mutx(xSemaphoreCreateMutex());

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}



static void Task1(void *pvParameters)
{

	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	vTaskDelay(10);
	char task[] = "Task 1 ";
	bool pressed = false;

	while(1)
	{
		if(sw1.read())
		{
			pressed = true;
			while(sw1.read()){};
		}
		if(pressed)
		{
			while(1)
			{
				xEventGroupSetBits(event, BT_1);
			}
		}
	}
}
static void Task2(void *pvParameters)
{

	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	vTaskDelay(10);

	char task[] = "Task 2 ";

	bool pressed = false;
	while(1)
	{
		if(sw2.read())
		{
			pressed = true;
			while(sw2.read()){}
		}
		if(pressed)
		{
			while(1)
			{
				xEventGroupSetBits(event, BT_2);
			}
		}
	}
}

static void Task3(void *pvParameters)
{

	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	vTaskDelay(10);

	int cnt = 0;
	char task[] = "Task 3 ";
	bool pressed = false;

	while(1)
	{
		if(sw3.read())
		{
			pressed = true;
			while(sw3.read()){}
		}
		if(pressed)
		{
			while(1)
			{
				xEventGroupSetBits(event, BT_3);
			}
		}
	}
}

static void Task4(void *pvParameters)
{

	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	vTaskDelay(10);
	int time = xTaskGetTickCount();
	char task[] = "Task 3 ";


	while(1)
	{

		if((xTaskGetTickCount() - time) >= 10000)
		{
			DEBUGOUT("FAIL\r\n");
			time = xTaskGetTickCount();
		}
		else
		{
			xEventGroupWaitBits(event, BITS, pdTRUE, pdTRUE, xTaskGetTickCount() - time);
			DEBUGOUT("OK %d ms\r\n", xTaskGetTickCount() - time);
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

	xTaskCreate(Task1, "Task1",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Task2",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task3, "Task3",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);
	xTaskCreate(Task4, "Task4",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

