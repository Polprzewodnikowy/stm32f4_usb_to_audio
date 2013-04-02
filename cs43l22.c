/*
 * cs43l22.c
 *
 *  Created on: 28-03-2013
 *      Author: Polprzewodnikowy
 */

#include "stm32f4xx.h"
#include "hdr/hdr_bitband.h"
#include "gpio.h"
#include "cs43l22.h"

void cs43l22_i2c_init(void);
void cs43l22_i2c_write(uint8_t addr, uint8_t data);

static uint32_t voidbuffer[2] = {0, 0};

uint16_t plli2scfgr[2][3] = {
		{3, 258, SPI_I2SPR_ODD | 3},	//96000
		{6, 258, SPI_I2SPR_ODD | 3},	//48000
};

void cs43l22_init(uint32_t freq)
{
	static uint32_t current_freq;
	uint8_t config;

	if(freq != 0)
		current_freq = freq;

	switch(current_freq)
	{
		case 96000:
			config = 0;
			break;
		case 48000:
			config = 1;
			break;
		default:
			config = 1;
			break;
	}

	cs43l22_i2c_init();

	gpio_pin_cfg(CS43L22_RESET_GPIO, CS43L22_RESET_PIN, GPIO_OUT_PP_2MHz);		//reset
	gpio_pin_cfg(CS43L22_WS_GPIO, CS43L22_WS_PIN, GPIO_AF6_PP_100MHz);			//ws
	gpio_pin_cfg(CS43L22_MCLK_GPIO, CS43L22_MCLK_PIN, GPIO_AF6_PP_100MHz);		//mclk
	gpio_pin_cfg(CS43L22_SCLK_GPIO, CS43L22_SCLK_PIN, GPIO_AF6_PP_100MHz);		//sclk
	gpio_pin_cfg(CS43L22_SDIN_GPIO, CS43L22_SDIN_PIN, GPIO_AF6_PP_100MHz);		//sdin

	CS43L22_RESET = 0;
	CS43L22_RESET = 1;

	cs43l22_i2c_write(0x04, 0b10101111);
	cs43l22_i2c_write(0x06, 0b00000111);

	cs43l22_i2c_write(0x00, 0x99);
	cs43l22_i2c_write(0x47, 0x80);
	cs43l22_i2c_write(0x32, (1<<7));
	cs43l22_i2c_write(0x32, 0x00);
	cs43l22_i2c_write(0x00, 0x00);

	RCC->PLLI2SCFGR = (plli2scfgr[config][0]<<28)	//PLLI2SR
					| (plli2scfgr[config][1]<<6);	//PLLI2SN
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	SPI3->CR2 |= SPI_CR2_TXDMAEN;
	SPI3->I2SCFGR = SPI_I2SCFGR_I2SMOD
				  | SPI_I2SCFGR_I2SCFG_1;
	SPI3->I2SPR = SPI_I2SPR_MCKOE
				| plli2scfgr[config][2];
	RCC->CR |= RCC_CR_PLLI2SON;

	while((RCC->CR & RCC_CR_PLLI2SRDY) == RESET);

	SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;

	cs43l22_i2c_write(0x02, 0x9E);

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream5->PAR = (uint32_t)&SPI3->DR;
	DMA1_Stream5->M0AR = (uint32_t)0;
	DMA1_Stream5->NDTR = (uint32_t)0;
	DMA1_Stream5->CR =	DMA_SxCR_CIRC
					  | DMA_SxCR_DIR_0
					  | DMA_SxCR_MINC
					  | DMA_SxCR_PSIZE_0
					  | DMA_SxCR_MSIZE_1
					  | DMA_SxCR_PL_1;
	DMA1_Stream5->FCR = DMA_SxFCR_FTH;

	return;
}

void cs43l22_deinit(void)
{
	cs43l22_stop();
}

void cs43l22_play(uint32_t *pbuf, uint32_t size)
{
	if((DMA1_Stream5->CR & DMA_SxCR_EN) && (SPI3->I2SCFGR & SPI_I2SCFGR_I2SE))
	{
		while(SPI3->SR & SPI_SR_CHSIDE);	//Sync channels
		while(!(SPI3->SR & SPI_SR_CHSIDE));
	}
	DMA1_Stream5->CR &= ~DMA_SxCR_EN;
	while(DMA1_Stream5->CR & DMA_SxCR_EN);
	DMA1_Stream5->M0AR = (uint32_t)pbuf;
	DMA1_Stream5->NDTR = (uint32_t)size;
	if(!(SPI3->I2SCFGR & SPI_I2SCFGR_I2SE))
	{
		SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;
		while(!(SPI3->I2SCFGR & SPI_I2SCFGR_I2SE));
	}
	DMA1_Stream5->CR |=	DMA_SxCR_EN;
}

void cs43l22_stop(void)
{
	DMA1_Stream5->CR &= ~DMA_SxCR_EN;
	while(DMA1_Stream5->CR & DMA_SxCR_EN);
	DMA1_Stream5->M0AR = (uint32_t)&voidbuffer;
	DMA1_Stream5->NDTR = (uint32_t)2;
	DMA1_Stream5->CR |=	DMA_SxCR_EN;
}

void cs43l22_volume(uint8_t vol)
{
	cs43l22_i2c_write(0x22, vol);
	cs43l22_i2c_write(0x23, vol);
}

void cs43l22_mute(uint8_t cmd)
{
	if(cmd)
		cs43l22_i2c_write(0x0F, 0b11110000);
	else
		cs43l22_i2c_write(0x0F, 0b00000000);
}

void cs43l22_i2c_init(void)
{
	gpio_pin_cfg(STM32_I2C_SDA_GPIO, STM32_I2C_SDA_PIN, GPIO_AF4_OD_2MHz_PULL_UP);	//sda
	gpio_pin_cfg(STM32_I2C_SCL_GPIO, STM32_I2C_SCL_PIN, GPIO_AF4_OD_2MHz_PULL_UP);	//scl
	RCC->STM32_I2C_APB |= STM32_I2C_APB_EN;
	STM32_I2C_PERI->CR1 = I2C_CR1_SWRST;
	STM32_I2C_PERI->CR1 = 0;
	STM32_I2C_PERI->CR2 = 42;
	STM32_I2C_PERI->CCR = I2C_CCR_FS | 35;//178;
	STM32_I2C_PERI->TRISE = 30;//45;
	STM32_I2C_PERI->CR1 = I2C_CR1_PE;
}

void cs43l22_i2c_write(uint8_t addr, uint8_t data)
{
	uint32_t dummy;

	STM32_I2C_PERI->CR1 |= I2C_CR1_START;						// request a start
	while((STM32_I2C_PERI->SR1 & I2C_SR1_SB) == RESET);			// wait for start to finish
	STM32_I2C_PERI->DR = 0x94;									// transfer address
	while((STM32_I2C_PERI->SR1 & I2C_SR1_ADDR) == RESET);		// wait for address transfer
	dummy = STM32_I2C_PERI->SR2;

	while ((STM32_I2C_PERI->SR1 & I2C_SR1_TXE) == RESET);		// wait for DR empty
	STM32_I2C_PERI->DR = addr;									// transfer one byte, increment pointer
	while ((STM32_I2C_PERI->SR1 & I2C_SR1_TXE) == RESET);
	STM32_I2C_PERI->DR = data;

	while((STM32_I2C_PERI->SR1 & I2C_SR1_TXE) == RESET || (STM32_I2C_PERI->SR1 & I2C_SR1_BTF) == SET);
																// wait for bus not-busy
	STM32_I2C_PERI->CR1 |= I2C_CR1_STOP;						// request a stop

	return;
}
