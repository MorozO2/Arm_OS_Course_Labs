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
#include <string>
// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "ITM_write.h"
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
SemaphoreHandle_t sbPIN;
QueueHandle_t pinQ;
extern "C" {
void PIN_INT0_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	//Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT, PININTCH0);
	int pin = 1;
	xQueueSendFromISR(pinQ, &pin, &xHigherPriorityWoken);
	// End the ISR and (possibly) do a context switch
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT1_IRQHandler(void)
{

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	//Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT, PININTCH0);
	int pin = 2;
	xQueueSendFromISR(pinQ, &pin, &xHigherPriorityWoken);
	// End the ISR and (possibly) do a context switch
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
void PIN_INT2_IRQHandler(void)
{

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	//Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT, PININTCH0);
	int pin = 3;
	xQueueSendFromISR(pinQ, &pin, &xHigherPriorityWoken);
	// End the ISR and (possibly) do a context switch
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

}


static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	Chip_GPIO_Init(LPC_GPIO);
	Chip_PININT_Init(LPC_GPIO_PIN_INT);


	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);

	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1 );
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);

	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);

	Chip_INMUX_PinIntSel(0, 0, 17);
	Chip_INMUX_PinIntSel(1, 1, 11);
	Chip_INMUX_PinIntSel(2, 1, 9);

}

void EnablePinINT_NVIC()
{
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);


	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority(PIN_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	NVIC_SetPriority(PIN_INT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	NVIC_SetPriority(PIN_INT2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}

static void Button_Task(void *pvParameters)
{
	EnablePinINT_NVIC();
	int buff = 0;
	int temp = 0;
	int cnt = 0;
	char bt1[] = "Button ";
	char pr[] = " was pressed ";
	char tm[] = " times\r\n";
	bool got = false;

	while(1)
	{

		if(xQueueReceive(pinQ, &buff, portMAX_DELAY))
		{
			if(buff != temp)
			{

				if(got)
				{
					Board_UARTPutSTR(bt1); Board_UARTPutSTR(std::to_string(temp).c_str());
					Board_UARTPutSTR(pr); Board_UARTPutSTR(std::to_string(cnt).c_str()); Board_UARTPutSTR(tm);
					got = false;
					cnt = 1;
				}
				temp = buff;
				got = true;

			}

			else if(buff == temp)
			{
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
 	pinQ = xQueueCreate(20, sizeof(int));

	xTaskCreate(Button_Task, "Button_Task",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

