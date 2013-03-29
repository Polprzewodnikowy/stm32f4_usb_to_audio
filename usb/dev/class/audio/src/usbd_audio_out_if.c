/**
  ******************************************************************************
  * @file    usbd_audio_out_if.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the Audio Out (palyback) interface API.
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
#include "usbd_audio_core.h"
#include "usbd_audio_out_if.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_audio_out_if 
  * @brief usbd out interface module
  * @{
  */ 

/** @defgroup usbd_audio_out_if_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup usbd_audio_out_if_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup usbd_audio_out_if_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup usbd_audio_out_if_Private_FunctionPrototypes
  * @{
  */
static uint8_t Init(uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
static uint8_t DeInit(uint32_t options);
static uint8_t AudioCmd(uint8_t* pbuf, uint32_t size, uint8_t cmd);
static uint8_t VolumeCtl(uint8_t vol);
static uint8_t MuteCtl(uint8_t cmd);
static uint8_t PeriodicTC(uint8_t cmd);
static uint8_t GetState(void);

/**
  * @}
  */ 

/** @defgroup usbd_audio_out_if_Private_Variables
  * @{
  */ 
AUDIO_FOPS_TypeDef AUDIO_OUT_fops =
{
	Init,
	DeInit,
	AudioCmd,
	VolumeCtl,
	MuteCtl,
	PeriodicTC,
	GetState
};

static uint8_t AudioState = AUDIO_STATE_INACTIVE;

/**
  * @}
  */ 

/** @defgroup usbd_audio_out_if_Private_Functions
  * @{
  */ 

/**
  * @brief  Init
  *         Initialize and configures all required resources for audio play function.
  * @param  AudioFreq: Statrtup audio frequency. 
  * @param  Volume: Startup volume to be set.
  * @param  options: specific options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t Init(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
	static uint32_t Initialized = 0;
	if(Initialized == 0)
	{
		cs43l22_init(AudioFreq);
		cs43l22_volume(Volume);
		Initialized = 1;
	}
	AudioState = AUDIO_STATE_ACTIVE;
	return AUDIO_OK;
}

/**
  * @brief  DeInit
  *         Free all resources used by low layer and stops audio-play function.
  * @param  options: options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t DeInit(uint32_t options)
{
	AudioState = AUDIO_STATE_INACTIVE;
	GPIOD->BSRRH = GPIO_BSRR_BS_12;
	GPIOD->BSRRH = GPIO_BSRR_BS_13;
	GPIOD->BSRRH = GPIO_BSRR_BS_14;
	return AUDIO_OK;
}

/**
  * @brief  AudioCmd 
  *         Play, Stop, Pause or Resume current file.
  * @param  pbuf: address from which file shoud be played.
  * @param  size: size of the current buffer/file.
  * @param  cmd: command to be executed, can be AUDIO_CMD_PLAY , AUDIO_CMD_PAUSE, 
  *              AUDIO_CMD_RESUME or AUDIO_CMD_STOP.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t AudioCmd(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
	switch(cmd)
	{

		case AUDIO_CMD_PLAY:
			cs43l22_play((uint32_t)pbuf, size);
			AudioState = AUDIO_STATE_PLAYING;
			GPIOD->BSRRL = GPIO_BSRR_BS_12;
			GPIOD->BSRRH = GPIO_BSRR_BS_13;
			break;
		case AUDIO_CMD_STOP:
			cs43l22_stop();
			AudioState = AUDIO_STATE_STOPPED;
			GPIOD->BSRRH = GPIO_BSRR_BS_12;
			GPIOD->BSRRH = GPIO_BSRR_BS_13;
			break;
		case AUDIO_CMD_PAUSE:
			if(AudioState == AUDIO_STATE_PLAYING)
			{
				cs43l22_stop();
				AudioState = AUDIO_STATE_PAUSED;
				GPIOD->BSRRH = GPIO_BSRR_BS_12;
				GPIOD->BSRRL = GPIO_BSRR_BS_13;
			}
			else if(AudioState == AUDIO_STATE_PAUSED)
			{
				cs43l22_play((uint32_t)pbuf, size);
				AudioState = AUDIO_STATE_PLAYING;
				GPIOD->BSRRL = GPIO_BSRR_BS_12;
				GPIOD->BSRRH = GPIO_BSRR_BS_13;
			}
			break;
		default:
			return AUDIO_FAIL;
			break;
	}
	return AUDIO_OK;
}

/**
  * @brief  VolumeCtl
  *         Set the volume level in %
  * @param  vol: volume level to be set in % (from 0% to 100%)
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t VolumeCtl(uint8_t vol)
{
	cs43l22_volume(vol);
	return AUDIO_OK;
}

/**
  * @brief  MuteCtl
  *         Mute or Unmute the audio current output
  * @param  cmd: can be 0 to unmute, or 1 to mute.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t MuteCtl(uint8_t cmd)
{
	if(cmd)
	{
		cs43l22_mute(cmd);
		GPIOD->BSRRL = GPIO_BSRR_BS_14;
	}
	else
	{
		cs43l22_mute(cmd);
		GPIOD->BSRRH = GPIO_BSRR_BS_14;
	}
	return AUDIO_OK;
}

/**
  * @brief  
  *         
  * @param  
  * @param  
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t PeriodicTC(uint8_t cmd)
{
	return AUDIO_OK;
}

/**
  * @brief  GetState
  *         Return the current state of the audio machine
  * @param  None
  * @retval Current State.
  */
static uint8_t GetState(void)
{
	return AudioState;
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
