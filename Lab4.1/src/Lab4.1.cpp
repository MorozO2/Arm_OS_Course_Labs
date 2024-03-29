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


SemaphoreHandle_t binar;


static void SwitchCheckTask(void *pvParameters)
{

	DigitalIoPin LimitSW1(0, 27, DigitalIoPin::pullup, true);
	DigitalIoPin LimitSW2(0, 28, DigitalIoPin::pullup, true);

	while(1)
	{
		if(LimitSW1.read() == true && xSemaphoreTake(binar, portMAX_DELAY))
		{
			Board_LED_Set(0, true);
			while(LimitSW1.read() == true){}
			xSemaphoreGive(binar);

		}


		else if(LimitSW2.read() == true && xSemaphoreTake(binar, portMAX_DELAY))
		{
			Board_LED_Set(1, true);
			while(LimitSW2.read() == true){}
			xSemaphoreGive(binar);
		}

		else
		{
			Board_LED_Set(0, false);
			Board_LED_Set(1, false);
		}

	}
}

static void StepTask(void *pvParameters)
{
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);

	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	DigitalIoPin SwitchDir(1, 0, DigitalIoPin::output, true);


	while(1)
	{

		step.write(false);

		while(sw1.read() == true && sw3.read() ==true) {}


		if(sw1.read() == true && xSemaphoreTake(binar, portMAX_DELAY))
		{
			SwitchDir.write(false); //TRUE RUNS CLOCKWISE
			step.write(true);
			xSemaphoreGive(binar);
			vTaskDelay(2);

		}
		else if(sw3.read() == true && xSemaphoreTake(binar, portMAX_DELAY))
		{
			SwitchDir.write(true); //FALSE RUNS COUNT.-CLOCKWISE
			step.write(true);
			xSemaphoreGive(binar);
			vTaskDelay(2);
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

	xTaskCreate(SwitchCheckTask, "SwitchCheckTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	xTaskCreate(StepTask, "StepTask",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

