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
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


SemaphoreHandle_t binar;

DigitalIoPin LimitSW1(0, 27, DigitalIoPin::pullup, true);
DigitalIoPin LimitSW2(0, 28, DigitalIoPin::pullup, true);

static void SwitchCheckTask(void *pvParameters)
{
	bool waiting =true;

	while(1)
	{
		while(waiting )
		{
			if(LimitSW1.read() == false && LimitSW2.read() == false) {waiting = false;}
			Board_LED_Set(2, true);
			vTaskDelay(100);
			Board_LED_Set(2, false);
			vTaskDelay(100);
		}

		xSemaphoreGive(binar);


		if(LimitSW1.read() )
		{
			Board_LED_Set(0, true);
			Board_LED_Set(1, false);
			vTaskDelay(1000);
		}

		if(LimitSW2.read() )
		{
			Board_LED_Set(1, true);
			Board_LED_Set(0, false);
			vTaskDelay(1000);
		}

		Board_LED_Set(0, false);
		Board_LED_Set(1, false);

	}
}

static void StepTask(void *pvParameters)
{

	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	DigitalIoPin SwitchDir(1, 0, DigitalIoPin::output, true);

	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);

	int cnt = 0;
	int distance = 0;
	bool countStarted = false;
	bool gotDist =  false;
	bool dir = false;
	bool cycleStart = false;
	SwitchDir.write(dir);

	while(1)
	{
		if(LimitSW1.read() && LimitSW2.read())
		{
			vTaskDelay(5000); //PAUSE FOR 5 SEC IN BOTH LIMIT SWITCHES ARE ON
			gotDist = false;
			cycleStart = false; //RESETS ALL FLAGS
		}

		else if(xSemaphoreTake(binar, portMAX_DELAY)) //CHECK IF SEMAPHORE IS AVAILABLE
		{

			if(gotDist) //CHECK IF DISTANCE IS CALCULATED
			{

				if(cycleStart)// CHECK IF CYCLE IS STARTED
				{
					int setDist = distance;
					for( ; setDist > 14 ; setDist--)
					{
						step.write(true);
						vTaskDelay(1);
						step.write(false);
						vTaskDelay(1);
						if(LimitSW1.read() && LimitSW2.read())//RESET
						{
							vTaskDelay(5000); //PAUSE FOR 5 SEC IN BOTH LIMIT SWITCHES ARE ON
							gotDist = false;
							cycleStart = false; //RESETS ALL FLAGS
						}
					}
					dir = !dir;
					SwitchDir.write(dir);
				}
				else//IF CYCLE IS NOT STARTED, TURN THE LIGHT ON, TAKE A FEW STEP AND START THE CYCLE
				{
					Board_LED_Set(2, true);
					vTaskDelay(2000);
					Board_LED_Set(2, false);

					for(int i = 0; i > 15; i--) //MOVES A FEW STEPS
					{
						step.write(true);
						vTaskDelay(1);
						step.write(false);
						vTaskDelay(1);
					}
					cycleStart = true;
				}

			}


			else if (LimitSW1.read() || LimitSW2.read())
				{
					if(countStarted) //COUNT STARTS IF ONE OF THE LIMIT SWITCHES WAS HIT ONCE
					{
						distance = cnt;
						gotDist = true;
						DEBUGOUT("Distance %d\r\n", distance);
						cnt = 0;
					}

					cnt = 0;
					countStarted = true;
					dir = !dir;
					SwitchDir.write(dir);
					for(int i = 5; i > 0; i--) //TAKES 3 STEPS
					{
						step.write(true);
						vTaskDelay(1);
						step.write(false);
						vTaskDelay(1);
						cnt++;
					}
				}

			else
			{
				step.write(true);
				vTaskDelay(1);
				step.write(false);
				vTaskDelay(1);
				cnt++;
			}
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

