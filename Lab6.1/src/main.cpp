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
#include <string>
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
	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


volatile uint32_t RIT_count;
SemaphoreHandle_t sbRIT;


DigitalIoPin LimitSW1(0, 27, DigitalIoPin::pullup, true);
DigitalIoPin LimitSW2(0, 28, DigitalIoPin::pullup, true);

extern "C" {
void RIT_IRQHandler(void)
{
 // This used to check if a context switch is required
 portBASE_TYPE xHigherPriorityWoken = pdFALSE;
 // Tell timer that we have processed the interrupt.
 // Timer then removes the IRQ until next match occurs
 Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
 if(RIT_count > 0) {
 RIT_count--;
 // do something useful here...
 }
 else {
 Chip_RIT_Disable(LPC_RITIMER); // disable timer
 // Give semaphore and set context switch flag if a higher priority task was woken up
 xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
 }
 // End the ISR and (possibly) do a context switch
 portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

void RIT_start(int count, int us)
{
 uint64_t cmp_value;
 // Determine approximate compare value based on clock rate and passed interval
 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
 // disable timer during configuration
 Chip_RIT_Disable(LPC_RITIMER);
 RIT_count = count;
 // enable automatic clear on when compare value==timer value
 // this makes interrupts trigger periodically
 Chip_RIT_EnableCompClear(LPC_RITIMER);
 // reset the counter
 Chip_RIT_SetCounter(LPC_RITIMER, 0);
 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
 // start counting
 Chip_RIT_Enable(LPC_RITIMER);
 // Enable the interrupt signal in NVIC (the interrupt controller)
 NVIC_EnableIRQ(RITIMER_IRQn);
 // wait for ISR to tell that we're done
 if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
 // Disable the interrupt signal in NVIC (the interrupt controller)
 NVIC_DisableIRQ(RITIMER_IRQn);
 }
 else {
 // unexpected error
 }
}


static void UserInputTask(void *pvParameters)
{
	std::string speed;
	std::string command;

	std::string right = "right";
	std::string left = "left";
	std::string pps = "pps";
	bool gotInput = false;

	int c = 0;
	while(1)
	{
		while((c = Board_UARTGetChar()) != EOF)
		{
			Board_UARTPutChar(c);

			if(c > 47 && c < 58) //	INPUTS THE VALUES FOR SPEED
			{
				speed.push_back(c);
			}
			else if(c == 13 && gotInput == false)
			{
				gotInput = true;
			}
			else if(c != 32) // INPUTS THE command
			{
				command.push_back(c);
			}
		}
		if(command == right && gotInput)
		{
			DEBUGOUT("\r\n%s %d\r\n", command.c_str(), atoi(speed.c_str()));
			speed.clear();
			command.clear();
			gotInput = false;
		}
		else if(command  == left && gotInput)
		{
			DEBUGOUT("\r\n%s %d\r\n", command.c_str(), atoi(speed.c_str()));
			speed.clear();
			command.clear();
			gotInput = false;
		}
		else if(command  == pps && gotInput)
		{
			DEBUGOUT("\r\n%s %d\r\n", command.c_str(), atoi(speed.c_str()));
			speed.clear();
			command.clear();
			gotInput = false;
		}
		else if(gotInput)
		{
			DEBUGOUT("\r\n error \r\n");
			speed.clear();
			command.clear();
			gotInput = false;
		}


	}
}

static void StepTask(void *pvParameters)
{

	while(1)
	{

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


	sbRIT = xSemaphoreCreateBinary();


	xTaskCreate(UserInputTask, "UserInputTask",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/*xTaskCreate(StepTask, "StepTask",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
*/
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

