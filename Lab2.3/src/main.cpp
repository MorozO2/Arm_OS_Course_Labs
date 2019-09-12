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
#include <string>

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

SemaphoreHandle_t countingSem;
SemaphoreHandle_t mutx;


static void askingTask(void *pvParameters)
{
	std::string you = "[You] ";
	int c;
	int count = 0;
	while(1)
	{
		while((c = Board_UARTGetChar()) != EOF)
		{
			//if(count == 60){c = 13;}//Checks if count is up to 60

			if(c!=13 && count < 60)
			{
				if(xSemaphoreTake(mutx, portMAX_DELAY))
				{
					Board_UARTPutChar(c);
					xSemaphoreGive(mutx);
					if(c == 63) {xSemaphoreGive(countingSem); you.erase(6, count);} // Check whether "?" was entered

				}

				you.push_back(c);
				count++;
			}
			else
			{

				if(xSemaphoreTake(mutx, portMAX_DELAY))
				{
					DEBUGOUT("\r%s\r\n", you.c_str());
					xSemaphoreGive(mutx);
				}
				you.erase(6, count);
				count = 0;

			}
		}
	}
}

static void OracleTask(void *pvParameters)
{

	std::string oracle = "[Oracle] ";
	std::string hm = "Hmm...";

	while(1)
	{

		if(xSemaphoreTake(countingSem, portMAX_DELAY))
		{

			if(xSemaphoreTake(mutx, portMAX_DELAY)) {DEBUGOUT("\r\n%s%s\r\n", oracle.c_str()), hm.c_str(); xSemaphoreGive(mutx);}
			vTaskDelay(3000);
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

	countingSem = xSemaphoreCreateCounting(3, 0);
	mutx = xSemaphoreCreateMutex();

	 //Gives initial semaphore for access to UART
	xTaskCreate(askingTask, "askingTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	xTaskCreate(OracleTask, "OracleTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);



	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

