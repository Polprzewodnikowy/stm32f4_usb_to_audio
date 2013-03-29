/** \file config.h
 * \brief Basic configuration of the project
 * \author Freddie Chopin, http://www.freddiechopin.info/
 * \date 2012-03-17
 */

/******************************************************************************
* project: stm32f4_blink_led
* chip: STM32F407VG
* compiler: arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.6.2
* 	20110921 (release) [ARM/embedded-4_6-branch revision 182083]
******************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include "hdr/hdr_bitband.h"

/*
+=============================================================================+
| global definitions
+=============================================================================+
*/

#define CRYSTAL								8000000ul	///< quartz crystal resonator which is connected to the chip
#define FREQUENCY							168000000ul	///< desired target frequency of the core

#define USER_bb								bitband_t m_BITBAND_PERIPH(&GPIOA->IDR, 0)
#define LEDG_bb								bitband_t m_BITBAND_PERIPH(&GPIOD->ODR, 12)
#define LEDO_bb								bitband_t m_BITBAND_PERIPH(&GPIOD->ODR, 13)
#define LEDR_bb								bitband_t m_BITBAND_PERIPH(&GPIOD->ODR, 14)
#define LEDB_bb								bitband_t m_BITBAND_PERIPH(&GPIOD->ODR, 15)

/*
+=============================================================================+
| strange variables
+=============================================================================+
*/

/*
+=============================================================================+
| global variables
+=============================================================================+
*/

/*
+=============================================================================+
| global functions' declarations
+=============================================================================+
*/

/******************************************************************************
* END OF FILE
******************************************************************************/
#endif /* CONFIG_H_ */
