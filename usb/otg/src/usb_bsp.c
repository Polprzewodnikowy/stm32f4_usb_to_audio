/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file is responsible to offer board support package and is
  *          configurable by user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "gpio.h"
#include "stm32f4xx.h"

/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_BSP
  * @brief This file is responsible to offer board support package
  * @{
  */ 

/** @defgroup USB_BSP_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USB_BSP_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 





/** @defgroup USB_BSP_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBH_BSP_Private_Variables
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBH_BSP_Private_FunctionPrototypes
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_BSP_Private_Functions
  * @{
  */ 


/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	pdev = pdev;
	//gpio_pin_cfg(GPIOA,  8, GPIO_AF10_PP_100MHz);	//SOF
	gpio_pin_cfg(GPIOA,  9, GPIO_AF10_PP_100MHz);	//VBUS
	gpio_pin_cfg(GPIOA, 10, GPIO_AF10_PP_100MHz);	//ID
	gpio_pin_cfg(GPIOA, 11, GPIO_AF10_PP_100MHz);	//DM
	gpio_pin_cfg(GPIOA, 12, GPIO_AF10_PP_100MHz);	//DP

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
}
/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Enabele USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	pdev = pdev;
	NVIC_SetPriority(OTG_FS_IRQn, 1);
	NVIC_EnableIRQ(OTG_FS_IRQn);
	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  speed : Full, Low 
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(uint32_t speed, uint8_t state)
{
	speed = speed;
	if(!state)
		GPIOC->BSRRL = (1<<0);
	else
		GPIOC->BSRRH = (1<<0);
}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  Speed : Full, Low 
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(uint32_t speed)
{
	speed = speed;
	gpio_pin_cfg(GPIOC, 0, GPIO_OUT_PP_50MHz_PULL_UP);
	USB_OTG_BSP_mDelay(200);
}

/**
  * @brief  USB_OTG_BSP_TimeInit
  *         Initialises delay unit Systick timer /Timer2
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_TimeInit(void)
{

}

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay(const uint32_t usec)
{
	TIM2->ARR = usec - 1;
	TIM2->PSC = 84 - 1;
	TIM2->EGR = TIM_EGR_UG;
	TIM2->CR1 = TIM_CR1_OPM | TIM_CR1_CEN;
	while(TIM2->CR1 & TIM_CR1_CEN);
	//uint32_t count = 0;
	//const uint32_t utime = (120 * usec / 7);
	//do
	//{
	//  if ( ++count > utime )
	//  {
	//    return ;
	//  }
	//}
	//while (1); 
}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay(const uint32_t msec)
{
    USB_OTG_BSP_uDelay(msec * 1000);    
}


/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_TimerIRQ(void)
{

} 

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
