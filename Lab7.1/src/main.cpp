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
#include "sct_15xx.h"
#include "sct_pwm_15xx.h"
#include "swm_15xx.h"
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
void SCT_Init(void)
{
 LPC_SCT0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
 LPC_SCT0->CTRL_L |= (12-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

 LPC_SCT0->MATCHREL[0].L = 10*100-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
 LPC_SCT0->MATCHREL[1].L = 5; // match 1 used for duty cycle (in 10 steps)
 LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
 LPC_SCT0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
 LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
 LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
 LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
 LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
 LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	Chip_SCT_Init(LPC_SCT0);
	SCT_Init();
	ITM_init();
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}







static void SCT_Task(void *pvParameters)
{

	DigitalIoPin sw1(0,17, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, 0, 3);
	while(1)
	{
		if(sw1.read())
		{
			LPC_SCT0->MATCHREL[1].L++;
			vTaskDelay(10);

		}
		if(sw3.read())
		{
			LPC_SCT0->MATCHREL[1].L--;
			vTaskDelay(10);
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


	xTaskCreate(SCT_Task, "SCT_Task",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

