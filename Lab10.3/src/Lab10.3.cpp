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

QueueHandle_t cmdQ(xQueueCreate(6, sizeof(std::string)));
SemaphoreHandle_t bin(xSemaphoreCreateBinary());
SemaphoreHandle_t mutx(xSemaphoreCreateMutex());
TimerHandle_t reload;

static void reloadCallBack(TimerHandle_t pxTimer);

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}



static void Task1(void *pvParameters)
{
	int c = 0;
	std::string cmd;
	bool gotCmd = false;
	while(1)
	{

		while((c = Board_UARTGetChar()) && c != 13)
		{

			xTimerReset(reload, portMAX_DELAY);
			if(xSemaphoreTake(mutx, portMAX_DELAY))
			{
				Board_UARTPutChar(c);
				xSemaphoreGive(mutx);
				cmd.push_back(c);
			}
		}

		if(cmd == "help")
		{
			cmd.clear();
			gotCmd = false;
			DEBUGOUT("Type 'help' to see this instruction screen\r\n"
					"Type 'interval <number>' to set LED toggle rate in seconds\r\n"
					"Type 'time' to get seconds elapsed since last LED toggle\r\n");

		}
		else if(cmd.substr(0, 8) == "interval")
		{
			cmd.clear();
			gotCmd = false;
			DEBUGOUT("%d", std::stoi(cmd.substr(cmd.find(" "), cmd.length() )) );
		}

		else if(cmd == "time")
		{
			cmd.clear();
			gotCmd = false;
		}
		else
		{

		}


	}
}
static void Task2(void *pvParameters)
{
	std::string gotCmd;
	int del = 5000;
	while(1)
	{
		if(xQueueReceive(cmdQ, &gotCmd, portMAX_DELAY))
		{
			if(xSemaphoreTake(mutx, portMAX_DELAY))
			{
				DEBUGOUT("\r\n%s", gotCmd.c_str()); xSemaphoreGive(mutx);
			}

			//vTaskDelay(del);

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

	reload = xTimerCreate("reloadCallBack", 30000 / portTICK_PERIOD_MS, pdTRUE, (void *)0, reloadCallBack);

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


static void reloadCallBack(TimerHandle_t pxTimer)
{
	xSemaphoreGive(bin);
}
