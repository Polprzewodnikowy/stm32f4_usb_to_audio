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

void cs43l22_init(uint32_t freq)
{
	static uint32_t current_freq;

	if(freq != 0)
		current_freq = freq;

	cs43l22_i2c_init();

	gpio_pin_cfg(CS43L22_RESET_GPIO, CS43L22_RESET_PIN, GPIO_OUT_PP_2MHz);		//reset
	gpio_pin_cfg(CS43L22_WS_GPIO, CS43L22_WS_PIN, GPIO_AF6_PP_100MHz);			//ws
	gpio_pin_cfg(CS43L22_MCLK_GPIO, CS43L22_MCLK_PIN, GPIO_AF6_PP_100MHz);		//mclk
	gpio_pin_cfg(CS43L22_SCLK_GPIO, CS43L22_SCLK_PIN, GPIO_AF6_PP_100MHz);		//sclk
	gpio_pin_cfg(CS43L22_SDIN_GPIO, CS43L22_SDIN_PIN, GPIO_AF6_PP_100MHz);		//sdin

	CS43L22_RESET = 0;
	CS43L22_RESET = 1;

	cs43l22_i2c_write(0x04, 0b10101111);
	//cs43l22_i2c_write(0x05, 0b10100000);
	cs43l22_i2c_write(0x06, 0b00000111);

	//cs43l22_volume(-12);

	cs43l22_i2c_write(0x00, 0x99);
	cs43l22_i2c_write(0x47, 0x80);
	cs43l22_i2c_write(0x32, (1<<7));
	cs43l22_i2c_write(0x32, 0x00);
	cs43l22_i2c_write(0x00, 0x00);

	RCC->PLLI2SCFGR = ((6<<28)|(258<<6));
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	SPI3->CR2 |= SPI_CR2_TXDMAEN;
	SPI3->I2SCFGR =	SPI_I2SCFGR_I2SMOD|SPI_I2SCFGR_I2SCFG_1;
	SPI3->I2SPR = SPI_I2SPR_MCKOE|SPI_I2SPR_ODD|3;
	RCC->CR |= RCC_CR_PLLI2SON;
	while((RCC->CR & RCC_CR_PLLI2SRDY) == RESET);
	SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;
	cs43l22_i2c_write(0x02, 0x9E);

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream5->PAR = (uint32_t)0x40003C0C;	//SPI3->DR, SPI3+0x0C
	DMA1_Stream5->M0AR = (uint32_t)0;
	DMA1_Stream5->NDTR = (uint32_t)0;
	DMA1_Stream5->CR =	DMA_SxCR_CIRC
						|DMA_SxCR_DIR_0
						|DMA_SxCR_MINC
						|DMA_SxCR_PSIZE_0
						|DMA_SxCR_MSIZE_1
						|DMA_SxCR_PL;
	DMA1_Stream5->FCR = DMA_SxFCR_FTH;

	return;
}

void cs43l22_play(uint32_t pbuf, uint32_t size)
{
	if((DMA1_Stream5->CR & DMA_SxCR_EN) && (SPI3->I2SCFGR & SPI_I2SCFGR_I2SE))
	{
		while(SPI3->SR & SPI_SR_CHSIDE);
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
	SPI3->I2SCFGR &= ~SPI_I2SCFGR_I2SE;
	cs43l22_init(0);
}

void cs43l22_volume(uint8_t vol)
{
	cs43l22_i2c_write(0x22, VOLUME_CONVERT(vol));
	cs43l22_i2c_write(0x23, VOLUME_CONVERT(vol));
}

void cs43l22_mute(uint8_t cmd)
{
	if(cmd)
		cs43l22_i2c_write(0x0F, 0b11110000);
	else
		cs43l22_i2c_write(0x0F, 0b00000000);
}

void cs43l22_i2c_write(uint8_t addr, uint8_t data)
{
	uint8_t addr_data[2];
	addr_data[0] = addr;
	addr_data[1] = data;
	i2c_write(0x94, addr_data, 2);
	return;
}

void cs43l22_i2c_init(void)
{
	gpio_pin_cfg(STM32_I2C_SDA_GPIO, STM32_I2C_SDA_PIN, GPIO_AF4_OD_2MHz_PULL_UP);	//sda
	gpio_pin_cfg(STM32_I2C_SCL_GPIO, STM32_I2C_SCL_PIN, GPIO_AF4_OD_2MHz_PULL_UP);	//scl
	RCC->STM32_I2C_APB |= STM32_I2C_APB_EN;
	STM32_I2C_PERI->CR1 = I2C_CR1_SWRST;
	STM32_I2C_PERI->CR1 = 0;
	STM32_I2C_PERI->CR2 = 42;
	STM32_I2C_PERI->CCR = 178;
	STM32_I2C_PERI->TRISE = 45;
	STM32_I2C_PERI->CR1 = I2C_CR1_PE;
}

void i2c_write(uint8_t address, uint8_t* data, uint32_t length)
{
	uint32_t dummy;

	STM32_I2C_PERI->CR1 |= I2C_CR1_START;						// request a start
	while((STM32_I2C_PERI->SR1 & I2C_SR1_SB) == RESET);			// wait for start to finish
	STM32_I2C_PERI->DR = address;								// transfer address
	while ((STM32_I2C_PERI->SR1 & I2C_SR1_ADDR) == RESET);		// wait for address transfer
	dummy = STM32_I2C_PERI->SR2;

	while(length--)												// transfer whole block
	{
		while ((STM32_I2C_PERI->SR1 & I2C_SR1_TXE) == RESET);	// wait for DR empty
		STM32_I2C_PERI->DR = *data++;							// transfer one byte, increment pointer
	}

	while((STM32_I2C_PERI->SR1 & I2C_SR1_TXE) == RESET || (STM32_I2C_PERI->SR1 & I2C_SR1_BTF) == SET);
																// wait for bus not-busy
	STM32_I2C_PERI->CR1 |= I2C_CR1_STOP;						// request a stop
}
