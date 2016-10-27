#include <stdbool.h>
#include <ucos_ii.h>
#include "gpioPin.h"

/*
*********************************************************************************************************
*                                            APPLICATION TASK PRIORITIES
*********************************************************************************************************
*/

typedef enum {
	APP_TASK_BUTTONS_PRIO = 4,
  APP_TASK_LED1_PRIO,
  APP_TASK_LED2_PRIO
} taskPriorities_t;

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  APP_TASK_BUTTONS_STK_SIZE           256
#define  APP_TASK_LED1_STK_SIZE              256
#define  APP_TASK_LED2_STK_SIZE              256

static OS_STK appTaskButtonsStk[APP_TASK_BUTTONS_STK_SIZE];
static OS_STK appTaskLED1Stk[APP_TASK_LED1_STK_SIZE];
static OS_STK appTaskLED2Stk[APP_TASK_LED2_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void appTaskButtons(void *pdata);
static void appTaskLED1(void *pdata);
static void appTaskLED2(void *pdata);

/*
*********************************************************************************************************
*                                            GLOBAL TYPES AND VARIABLES 
*********************************************************************************************************
*/


typedef enum {
	LED1 = 0, LED2,
	JLEFT, JRIGHT,
	JUP, JDOWN, N_DEVICES
} deviceNames_t;

enum {
	FLASH_MIN_DELAY     = 1,
	FLASH_INITIAL_DELAY = 500,
	FLASH_MAX_DELAY     = 1000,
	FLASH_DELAY_STEP    = 50
};

bool buttonPressedAndReleased(deviceNames_t button);
void incDelay(void);
void decDelay(void);

static gpioPin_t pin[N_DEVICES];
static bool flashing = false;
static int32_t flashingDelay = FLASH_INITIAL_DELAY;

/*
*********************************************************************************************************
*                                            GLOBAL FUNCTION DEFINITIONS
*********************************************************************************************************
*/

int main() {

  /* Initialise the GPIO pins */
	gpioPinInit(&pin[LED1],   1, 18, OUTPUT_PIN);
	gpioPinInit(&pin[LED2],   0, 13, OUTPUT_PIN);
	gpioPinInit(&pin[JLEFT],  5, 0,  INPUT_PIN);
	gpioPinInit(&pin[JRIGHT], 5, 4,  INPUT_PIN);
	gpioPinInit(&pin[JUP],    5, 2,  INPUT_PIN);
	gpioPinInit(&pin[JDOWN],  5, 1,  INPUT_PIN);

  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskButtons,                               
               (void *)0,
               (OS_STK *)&appTaskButtonsStk[APP_TASK_BUTTONS_STK_SIZE - 1],
               APP_TASK_BUTTONS_PRIO);
  
  OSTaskCreate(appTaskLED1,                               
               (void *)0,
               (OS_STK *)&appTaskLED1Stk[APP_TASK_LED1_STK_SIZE - 1],
               APP_TASK_LED1_PRIO);
  
  OSTaskCreate(appTaskLED2,                               
               (void *)0,
               (OS_STK *)&appTaskLED2Stk[APP_TASK_LED2_STK_SIZE - 1],
               APP_TASK_LED2_PRIO);

  
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

static void appTaskButtons(void *pdata) {
  /* Start the OS ticker -- must be done in the highest priority task */
  SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);
  
  /* Task main loop */
  while (true) {
    if (buttonPressedAndReleased(JRIGHT)) {
			flashing = true;
		}
		else if (buttonPressedAndReleased(JLEFT)) {
			flashing = false;
		}
		else if (flashing && buttonPressedAndReleased(JUP)) {
			incDelay();
		}
		else if (flashing && buttonPressedAndReleased(JDOWN)) {
			decDelay();
		}
    OSTimeDlyHMSM(0,0,0,100);
  }
}

static void appTaskLED1(void *pdata) {
  while (true) {
		if (flashing) {
      gpioPinToggle(&pin[LED1]);
		}
    OSTimeDly(flashingDelay);
  }
}


static void appTaskLED2(void *pdata) {
  while (true) {
		if (flashing) {
      gpioPinToggle(&pin[LED2]);
		}
    OSTimeDly(flashingDelay);
  } 
}

/*
 * @brief buttonPressedAndReleased(button) tests to see if the button has
 *        been pressed then released.
 *        
 * @param button - the name of the button
 * @result - true if button pressed then released, otherwise false
 *
 * If the value of the button's pin is 0 then the button is being pressed,
 * just remember this in savedState.
 * If the value of the button's pin is 1 then the button is released, so
 * if the savedState of the button is 0, then the result is true, otherwise
 * the result is false.
 */
bool buttonPressedAndReleased(deviceNames_t button) {
	bool result = false;
	uint32_t state;
	static uint32_t savedState[N_DEVICES] = {1,1,1,1,1,1}; 
	
	state = gpioPinVal(&pin[button]);
  if ((savedState[button] == 0) && (state == 1)) {
		result = true;
	}
	savedState[button] = state;
	return result;
}

void incDelay(void) {
	if (flashingDelay + FLASH_DELAY_STEP > FLASH_MAX_DELAY) {
		flashingDelay = FLASH_MAX_DELAY;
	}
	else {
		flashingDelay += FLASH_DELAY_STEP;
	}
}

void decDelay(void) {
	if (flashingDelay - FLASH_DELAY_STEP < FLASH_MIN_DELAY) {
		flashingDelay = FLASH_MIN_DELAY;
	}
	else {
		flashingDelay -= FLASH_DELAY_STEP;
	}
}
	
