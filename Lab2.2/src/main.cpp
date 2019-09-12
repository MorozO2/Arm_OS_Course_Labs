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


SemaphoreHandle_t binar;

//TASK FOR READING FROM UART AND ECHOING
static void readingTask(void *pvParameters)
{
	int c;
	while(1)
	{
		while( (c = Board_UARTGetChar())!= EOF)
		{

			Board_UARTPutChar( (char) c);
			xSemaphoreGive(binar); //ADDS TO SEMAPHORE, INDICATING THAT THE LIGHT CAN BE BLINKED
		}

	}
}
//TASK FOR INDICATING WITH LED THAT THE KEY WAS PRESSED
static void indicatingTask(void *pvParameters)
{

	while(1)
	{
		if(xSemaphoreTake(binar, portMAX_DELAY)) //CHECK IF SEMAPHORE IS AVAILABLE.
		{
			Board_LED_Set(0, true);			//BLINK IF IT IS AVAILABLE
			vTaskDelay(100);
			Board_LED_Set(0, false);
			vTaskDelay(100);

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
	xTaskCreate(readingTask, "readingTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(indicatingTask, "indicatingTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);



	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

