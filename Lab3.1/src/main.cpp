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
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

QueueHandle_t q;
SemaphoreHandle_t binar;
static void TypeInTask(void *pvParameters)
{
	int c;
	int count = 0;
	while(1)
	{
		while( (c = Board_UARTGetChar())!= EOF)
		{

			if(xSemaphoreTake(binar, portMAX_DELAY))
			{
				Board_UARTPutChar(c);
				xSemaphoreGive(binar);
			}

			if(c!=13){count++;}
			else if(c == 13)
			{
				xQueueSend(q, &count, portMAX_DELAY);

				if(xSemaphoreTake(binar, portMAX_DELAY))
				{
					DEBUGOUT("\r\n%d has been sent to queue\r\n", count);
					xSemaphoreGive(binar);
				}

				count = 0;
			}
		}

	}
}
//TASK FOR INDICATING WITH LED THAT THE KEY WAS PRESSED
static void EnderTask(void *pvParameters)
{
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	vTaskDelay(10);
	int end = -1;

	while(1)
	{
		if(sw1.read() == true)
		{
			xQueueSendToBack(q, &end, portMAX_DELAY);
			while(sw1.read() == true){}
		}
	}
}

static void AdderUpTask(void *pvParameters)
{
	int sum = 0;
	int receive = 0;
	while(1)
	{

		xQueueReceive(q, &receive, portMAX_DELAY);
		if(receive != -1)
		{
			sum += receive;
		}
		else
		{
			if(xSemaphoreTake(binar, portMAX_DELAY))
			{
				printf("You have typed %d characters\r\n", sum);
				xSemaphoreGive(binar);
			}

			sum = 0;
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
	binar = xSemaphoreCreateBinary();
	xSemaphoreGive(binar);
	q = xQueueCreate(5, sizeof(int));
	xTaskCreate(TypeInTask, "TypeInTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(EnderTask, "EnderTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	xTaskCreate(AdderUpTask, "AdderUpTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);



	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

