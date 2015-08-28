#include "gpioPin.h"

void gpioPinInit(gpioPin_t *pin, uint32_t portN, uint32_t pinN, gpioPinDir_t dir) {
	pin->pcon = (__IO uint32_t *)(LPC_IOCON_BASE + (portN * 0x80) + (4 * pinN));
	*pin->pcon &= ~(7UL << 0); /* select GPIO function for pin */
	*pin->pcon &= ~(3UL << 3); /* select mode: no pull-up/pull-down resistor enabled */
	pin->port = (LPC_GPIO_TypeDef *)(LPC_GPIO0_BASE + (portN * 0x20));
	pin->mask = (1UL << pinN);
	if (dir == OUTPUT_PIN) {
	  pin->port->DIR |= pin->mask;
	}
	else {
		pin->port->DIR &= ~pin->mask;
	}
}

void gpioPinSet(gpioPin_t *pin) {
	pin->port->SET = pin->mask;
}

void gpioPinClr(gpioPin_t *pin) {
	pin->port->CLR = pin->mask;
}

void gpioPinToggle(gpioPin_t *pin) {
	pin->port->PIN ^= pin->mask;
}

uint32_t gpioPinVal(gpioPin_t *pin) {
	return (pin->port->PIN & pin->mask ? 1 : 0);
}
