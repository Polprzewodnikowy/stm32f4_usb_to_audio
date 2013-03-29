/** \file gpio.c
 * \brief GPIO driver.
 * \details GPIO default configuration and function for configuring a pin
 * \author Freddie Chopin, http://www.freddiechopin.info/
 * \date 2012-03-17
 */

/******************************************************************************
* chip: STM32F4x
* compiler: arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.6.2
* 	20110921 (release) [ARM/embedded-4_6-branch revision 182083]
*
* prefix: gpio_
*
* available global functions:
* 	void gpio_init(void)
* 	void gpio_pin_cfg(GPIO_TypeDef *port_ptr, uint32_t pin, uint32_t configuration)
*
* available local functions:
*
* available interrupt handlers:
******************************************************************************/

/*
+=============================================================================+
| includes
+=============================================================================+
*/

#include <stdint.h>

#include "inc/stm32f4xx.h"

#include "hdr/hdr_gpio.h"

#include "gpio.h"

/*
+=============================================================================+
| local definitions
+=============================================================================+
*/

#define GPIO_GET_MODER(combination)			(((combination) & 0xF) >> 0)
#define GPIO_GET_OTYPER(combination)		(((combination) & 0xF0) >> 4)
#define GPIO_GET_OSPEEDR(combination)		(((combination) & 0xF00) >> 8)
#define GPIO_GET_PUPDR(combination)			(((combination) & 0xF000) >> 12)
#define GPIO_GET_AFR(combination)			(((combination) & 0xF0000) >> 16)
/*
+=============================================================================+
| module variables
+=============================================================================+
*/

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/

/*
+=============================================================================+
| global functions
+=============================================================================+
*/

/*------------------------------------------------------------------------*//**
* \brief GPIO initialization.
* \details Enables all GPIO ports.
*//*-------------------------------------------------------------------------*/

void gpio_init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
			RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN |
			RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN |
			RCC_AHB1ENR_GPIOIEN;			// enable all possible GPIO ports
}

/*------------------------------------------------------------------------*//**
* \brief Configures pin.
* \details Configures one pin in one port.
*
* \param [in] port_ptr points to the configuration structure of desired port
* \param [in] pin selects one pin, [0; 15]
* \param [in] configuration is a combined value of MODER, OTYPER, OSPEEDR,
* PUPDR and AFRx register bitfields, allowed values
* {GPIO_IN_FLOATING, GPIO_IN_PULL_UP, GPIO_IN_PULL_DOWN,
* GPIO_OUT_{PP, OD}_{2MHz, 25MHz, 50MHz, 100MHz},
* GPIO_OUT_{PP, OD}_{2MHz, 25MHz, 50MHz, 100MHz}_{PULL_UP, PULL_DOWN},
* GPIO_[AF0; AF15]_{PP, OD}_{2MHz, 25MHz, 50MHz, 100MHz},
* GPIO_[AF0; AF15]_{PP, OD}_{2MHz, 25MHz, 50MHz, 100MHz}_{PULL_UP, PULL_DOWN},
* GPIO_ANALOG}
*//*-------------------------------------------------------------------------*/

void gpio_pin_cfg(GPIO_TypeDef *port_ptr, uint32_t pin, uint32_t configuration)
{
	uint32_t moder, otyper, ospeedr, pupdr, afr, afrx;

	moder = port_ptr->MODER;				// localize
	moder &= ~(GPIO_MODER_mask << (pin * 2));	// clear current setting
	moder |= (GPIO_GET_MODER(configuration) << (pin * 2));	// apply new setting
	port_ptr->MODER = moder;				// save back

	otyper = port_ptr->OTYPER;				// localize
	otyper &= ~(GPIO_OTYPER_mask << pin);	// clear current setting
	otyper |= (GPIO_GET_OTYPER(configuration) << pin);	// apply new setting
	port_ptr->OTYPER = otyper;				// save back

	ospeedr = port_ptr->OSPEEDR;				// localize
	ospeedr &= ~(GPIO_OSPEEDR_mask << (pin * 2));	// clear current setting
	ospeedr |= (GPIO_GET_OSPEEDR(configuration) << (pin * 2));	// apply new setting
	port_ptr->OSPEEDR = ospeedr;				// save back

	pupdr = port_ptr->PUPDR;				// localize
	pupdr &= ~(GPIO_PUPDR_mask << (pin * 2));	// clear current setting
	pupdr |= (GPIO_GET_PUPDR(configuration) << (pin * 2));	// apply new setting
	port_ptr->PUPDR = pupdr;				// save back

	if (pin < 8)							// determine which AFR register should be used
		afrx = 0;							// AFRL - pins [0; 7]
	else
	{
		afrx = 1;							// AFRH - pins [8; 15]
		pin -= 8;							// trim pin number
	}

	afr = port_ptr->AFR[afrx];				// localize
	afr &= ~(GPIO_AFRx_mask << (pin * 4));	// clear current setting
	afr |= (GPIO_GET_AFR(configuration) << (pin * 4));	// apply new setting
	port_ptr->AFR[afrx] = afr;				// save back

}

/*
+=============================================================================+
| local functions
+=============================================================================+
*/

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/

/******************************************************************************
* END OF FILE
******************************************************************************/
