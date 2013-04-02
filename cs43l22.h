/*
 * cs43l22.h
 *
 *  Created on: 28-03-2013
 *      Author: Polprzewodnikowy
 */

#ifndef CS43L22_H_
#define CS43L22_H_

#define CS43L22_WS_GPIO		GPIOA
#define CS43L22_MCLK_GPIO	GPIOC
#define CS43L22_SCLK_GPIO	GPIOC
#define CS43L22_SDIN_GPIO	GPIOC
#define CS43L22_RESET_GPIO	GPIOD

#define CS43L22_WS_PIN		4
#define CS43L22_MCLK_PIN	7
#define CS43L22_SCLK_PIN	10
#define CS43L22_SDIN_PIN	12
#define CS43L22_RESET_PIN	4

#define CS43L22_RESET		bitband_t m_BITBAND_PERIPH(&CS43L22_RESET_GPIO->ODR, CS43L22_RESET_PIN)

#define STM32_I2C_PERI		I2C1
#define STM32_I2C_APB		APB1ENR
#define STM32_I2C_APB_EN	RCC_APB1ENR_I2C1EN

#define STM32_I2C_SDA_GPIO	GPIOB
#define STM32_I2C_SCL_GPIO	GPIOB

#define STM32_I2C_SDA_PIN	9
#define STM32_I2C_SCL_PIN	6

#define VOLUME_CONVERT(Volume)	((Volume > 100) ? 100 : ((uint8_t)((Volume * 255) / 100)))

void cs43l22_init(uint32_t freq);
void cs43l22_deinit(void);
void cs43l22_play(uint32_t *pbuf, uint32_t size);
void cs43l22_stop(void);
void cs43l22_volume(uint8_t vol);
void cs43l22_mute(uint8_t cmd);

#endif /* CS43L22_H_ */
