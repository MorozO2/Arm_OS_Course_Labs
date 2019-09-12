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
#include <time.h>
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
static void SendNumTask(void *pvParameters)
{
	int c;
	int randTime = 0;
	int randSend = 0;

	while(1)
	{
		randTime= rand() % 400 + 100;
		randSend = rand () % 100 + 1;
		xQueueSendToBack(q, &randSend, portMAX_DELAY);
		vTaskDelay(randTime);

	}
}

static void Send112Task(void *pvParameters)
{
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	vTaskDelay(10);
	int sendHelp = 112;
	while(1)
	{
		if(sw1.read() == true)
		{
			xQueueSendToFront(q, &sendHelp, portMAX_DELAY);
			while(sw1.read() == true){}
		}
	}
}

static void Check112Task(void *pvParameters)
{
	int receive = 0;
	while(1)
	{

		xQueueReceive(q, &receive, portMAX_DELAY);

		if(receive == 112)
		{
			printf("%d  -  Help me\r\n", receive);
			vTaskDelay(300);
		}

		else
		{
			printf("%d\r\n", receive);
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
	q = xQueueCreate(20, sizeof(int));
	xTaskCreate(SendNumTask, "SendNumTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(Send112Task, "EnderTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	xTaskCreate(Check112Task, "Check112Task",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);



	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

