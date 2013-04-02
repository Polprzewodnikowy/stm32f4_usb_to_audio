/*
 * usbd_audio.c
 *
 *  Created on: 31-03-2013
 *      Author: Polprzewodnikowy
 */

#include "usbd_audio.h"

uint8_t IsocOutBuff[TOTAL_OUT_BUF_SIZE * 2];
uint8_t *IsocOutWrPtr = IsocOutBuff;
uint8_t *IsocOutRdPtr = IsocOutBuff;
static __IO uint32_t usbd_audio_AltSet = 0;
static uint32_t PlayFlag = 0;
static uint32_t MuteFlag = 0;
static uint8_t CurrentVolume = 0;

uint8_t AC_data[64];
uint8_t AC_bRequest = 0;
uint16_t AC_wValue = 0;
uint16_t AC_wIndex = 0;
uint16_t AC_wLength = 0;

static uint8_t usbd_audio_Init(void *pdev, uint8_t cfgidx);
static uint8_t usbd_audio_DeInit(void *pdev, uint8_t cfgidx);
static uint8_t usbd_audio_Setup(void *pdev, USB_SETUP_REQ *req);
static uint8_t usbd_audio_EP0_TxSent(void *pdev);
static uint8_t usbd_audio_EP0_RxReady(void *pdev);
static uint8_t usbd_audio_DataIn(void *pdev, uint8_t epnum);
static uint8_t usbd_audio_DataOut(void *pdev, uint8_t epnum);
static uint8_t usbd_audio_SOF(void *pdev);
static uint8_t usbd_audio_IsoINIncomplete(void *pdev);
static uint8_t usbd_audio_IsoOUTIncomplete(void *pdev);
static uint8_t *usbd_audio_GetConfigDescriptor(uint8_t speed, uint16_t *length);

static void usbd_audio_ReqSetCur(void *pdev, USB_SETUP_REQ *req);
static void usbd_audio_ReqGetCur(void *pdev, USB_SETUP_REQ *req);
static void usbd_audio_ReqGetMin(void *pdev, USB_SETUP_REQ *req);
static void usbd_audio_ReqGetMax(void *pdev, USB_SETUP_REQ *req);
static void usbd_audio_ReqGetRes(void *pdev, USB_SETUP_REQ *req);

USBD_Class_cb_TypeDef AUDIO_cb =
{
	usbd_audio_Init,
	usbd_audio_DeInit,
	usbd_audio_Setup,
	usbd_audio_EP0_TxSent,
	usbd_audio_EP0_RxReady,
	usbd_audio_DataIn,
	usbd_audio_DataOut,
	usbd_audio_SOF,
	usbd_audio_IsoINIncomplete,
	usbd_audio_IsoOUTIncomplete,
	usbd_audio_GetConfigDescriptor,
};

static uint8_t usbd_audio_ConfigDescriptor[109] =
{
	/* Configuration 1 */
	0x09, /* bLength */
	USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
	LOBYTE(sizeof(usbd_audio_ConfigDescriptor)), /* wTotalLength  109 bytes*/
	HIBYTE(sizeof(usbd_audio_ConfigDescriptor)),
	0x02, /* bNumInterfaces */
	0x01, /* bConfigurationValue */
	0x00, /* iConfiguration */
	0xC0, /* bmAttributes  BUS Powred*/
	0x32, /* bMaxPower = 100 mA*/
	/* 09 byte*/

	/* USB Speaker Standard interface descriptor */
	AUDIO_INTERFACE_DESC_SIZE, /* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	0x00, /* bInterfaceNumber */
	0x00, /* bAlternateSetting */
	0x00, /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO, /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOCONTROL, /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
	0x00, /* iInterface */
	/* 09 byte*/

	/* USB Speaker Class-specific AC Interface Descriptor */
	AUDIO_INTERFACE_DESC_SIZE, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_CONTROL_HEADER, /* bDescriptorSubtype */
	0x00, /* 1.00 *//* bcdADC */
	0x01,
	0x27, /* wTotalLength = 39*/
	0x00,
	0x01, /* bInCollection */
	0x01, /* baInterfaceNr */
	/* 09 byte*/

	/* USB Speaker Input Terminal Descriptor */
	AUDIO_INPUT_TERMINAL_DESC_SIZE, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_CONTROL_INPUT_TERMINAL, /* bDescriptorSubtype */
	0x01, /* bTerminalID */
	0x01, /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
	0x01,
	0x00, /* bAssocTerminal */
	0x01, /* bNrChannels */
	0x00, /* wChannelConfig 0x0000  Mono */
	0x00,
	0x00, /* iChannelNames */
	0x00, /* iTerminal */
	/* 12 byte*/

	/* USB Speaker Audio Feature Unit Descriptor */
	0x09, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_CONTROL_FEATURE_UNIT, /* bDescriptorSubtype */
	AUDIO_OUT_STREAMING_CTRL, /* bUnitID */
	0x01, /* bSourceID */
	0x01, /* bControlSize */
	0x01, /* bmaControls(0) */
	0x02, /* bmaControls(1) */
	0x00, /* iTerminal */
	/* 09 byte*/

	/*USB Speaker Output Terminal Descriptor */
	0x09, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_CONTROL_OUTPUT_TERMINAL, /* bDescriptorSubtype */
	0x03, /* bTerminalID */
	0x01, /* wTerminalType  0x0301*/
	0x03,
	0x00, /* bAssocTerminal */
	0x02, /* bSourceID */
	0x00, /* iTerminal */
	/* 09 byte*/

	/* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
	/* Interface 1, Alternate Setting 0                                             */
	AUDIO_INTERFACE_DESC_SIZE, /* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	0x01, /* bInterfaceNumber */
	0x00, /* bAlternateSetting */
	0x00, /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO, /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
	0x00, /* iInterface */
	/* 09 byte*/

	/* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
	/* Interface 1, Alternate Setting 1                                           */
	AUDIO_INTERFACE_DESC_SIZE, /* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	0x01, /* bInterfaceNumber */
	0x01, /* bAlternateSetting */
	0x01, /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO, /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
	0x00, /* iInterface */
	/* 09 byte*/

	/* USB Speaker Audio Streaming Interface Descriptor */
	AUDIO_STREAMING_INTERFACE_DESC_SIZE, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_STREAMING_GENERAL, /* bDescriptorSubtype */
	0x01, /* bTerminalLink */
	0x01, /* bDelay */
	0x01, /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
	0x00,
	/* 07 byte*/

	/* USB Speaker Audio Type III Format Interface Descriptor */
	0x0B, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_STREAMING_FORMAT_TYPE, /* bDescriptorSubtype */
	AUDIO_FORMAT_TYPE_III, /* bFormatType */
	0x02, /* bNrChannels */
	0x02, /* bSubFrameSize :  2 Bytes per frame (16bits) */
	16, /* bBitResolution (16-bits per sample) */
	0x01, /* bSamFreqType only one frequency supported */
	SAMPLE_FREQ(USBD_AUDIO_FREQ), /* Audio sampling frequency coded on 3 bytes */
	/* 11 byte*/

	/* Endpoint 1 - Standard Descriptor */
	AUDIO_STANDARD_ENDPOINT_DESC_SIZE, /* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_OUT_EP, /* bEndpointAddress 1 out endpoint*/
	USB_ENDPOINT_TYPE_ISOCHRONOUS, /* bmAttributes */
	AUDIO_PACKET_SZE(USBD_AUDIO_FREQ), /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
	0x01, /* bInterval */
	0x00, /* bRefresh */
	0x00, /* bSynchAddress */
	/* 09 byte*/

	/* Endpoint - Audio Streaming Descriptor*/
	AUDIO_STREAMING_ENDPOINT_DESC_SIZE, /* bLength */
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType */
	AUDIO_ENDPOINT_GENERAL, /* bDescriptor */
	0x00, /* bmAttributes */
	0x00, /* bLockDelayUnits */
	0x00, /* wLockDelay */
	0x00,
	/* 07 byte*/
};

static uint8_t usbd_audio_Init(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Open(pdev, AUDIO_OUT_EP, AUDIO_OUT_PACKET, USB_OTG_EP_ISOC);
	cs43l22_init(USBD_AUDIO_FREQ);
	cs43l22_volume(CurrentVolume);
	DCD_EP_PrepareRx(pdev, AUDIO_OUT_EP, (uint8_t *)IsocOutBuff, AUDIO_OUT_PACKET);
	return USBD_OK;
}

static uint8_t usbd_audio_DeInit(void *pdev, uint8_t cfgidx)
{
	cfgidx = cfgidx;
	DCD_EP_Close(pdev, AUDIO_OUT_EP);
	cs43l22_deinit();
	return USBD_OK;
}

static uint8_t usbd_audio_Setup(void *pdev, USB_SETUP_REQ *req)
{
	uint16_t len = USB_AUDIO_DESC_SIZ;
	uint8_t *pbuf = usbd_audio_ConfigDescriptor + 18;

	switch(req->bmRequest & USB_REQ_TYPE_MASK)
	{
		case USB_REQ_TYPE_CLASS:
			switch(req->bRequest)
			{
				case AUDIO_REQ_SET_CUR:
					usbd_audio_ReqSetCur(pdev, req);
					break;
				case AUDIO_REQ_GET_CUR:
					usbd_audio_ReqGetCur(pdev, req);
					break;
				case 0x82:	//GET_MIN
					usbd_audio_ReqGetMin(pdev, req);
					break;
				case 0x83:	//GET_MAX
					usbd_audio_ReqGetMax(pdev, req);
					break;
				case 0x84:	//GET_RES
					usbd_audio_ReqGetRes(pdev, req);
					break;
				default:
					LEDR_bb ^= 1;
					USBD_CtlError(pdev, req);
					return USBD_FAIL;
			}
			break;
		case USB_REQ_TYPE_STANDARD:
			switch(req->bRequest)
			{
				case USB_REQ_GET_DESCRIPTOR:
					if((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
					{
						pbuf = usbd_audio_ConfigDescriptor + 18;
						len = MIN(USB_AUDIO_DESC_SIZ, req->wLength);
					}
					USBD_CtlSendData(pdev, pbuf, len);
					break;
				case USB_REQ_GET_INTERFACE:
					USBD_CtlSendData(pdev, (uint8_t *)&usbd_audio_AltSet, 1);
					break;
				case USB_REQ_SET_INTERFACE:
					if((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
						usbd_audio_AltSet = (uint8_t)(req->wValue);
					else
						USBD_CtlError(pdev, req);
					break;
				default:
					LEDO_bb ^= 1;
					break;
			}
	}
	return USBD_OK;
}

static uint8_t usbd_audio_EP0_TxSent(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_audio_EP0_RxReady(void *pdev)
{
	pdev = pdev;
	if(AC_bRequest == AUDIO_REQ_SET_CUR)
	{
		if(HIBYTE(AC_wIndex) == AUDIO_OUT_STREAMING_CTRL)
		{
			if(HIBYTE(AC_wValue) == 0x01)	//Mute
			{
				MuteFlag = AC_data[0];
				cs43l22_mute(AC_data[0]);
			}
			if(HIBYTE(AC_wValue) == 0x02)	//Volume
			{
				CurrentVolume = (AC_data[1]<<8 | AC_data[0]) / 256;
				cs43l22_volume(CurrentVolume);
			}
		}
	}
	AC_bRequest = 0;
	AC_wValue = 0;
	AC_wIndex = 0;
	AC_wLength = 0;
	return USBD_OK;
}

static uint8_t usbd_audio_DataIn(void *pdev, uint8_t epnum)
{
	pdev = pdev;
	epnum = epnum;
	return USBD_OK;
}

static uint8_t usbd_audio_DataOut(void *pdev, uint8_t epnum)
{
	if(epnum == AUDIO_OUT_EP)
	{
		if(IsocOutWrPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
			IsocOutWrPtr = IsocOutBuff;
		else
			IsocOutWrPtr += AUDIO_OUT_PACKET;

		((USB_OTG_CORE_HANDLE *)pdev)->dev.out_ep[epnum].even_odd_frame = (((USB_OTG_CORE_HANDLE *)pdev)->dev.out_ep[epnum].even_odd_frame) ? 0 : 1;
		DCD_EP_PrepareRx(pdev, AUDIO_OUT_EP, (uint8_t *)IsocOutWrPtr, AUDIO_OUT_PACKET);
		if((PlayFlag == 0) && (IsocOutWrPtr >= (IsocOutBuff + ((AUDIO_OUT_PACKET * OUT_PACKET_NUM) / 2))))
			PlayFlag = 1;
	}
	return USBD_OK;
}

static uint8_t usbd_audio_SOF(void *pdev)
{
	pdev = pdev;
	if(PlayFlag)
	{
		cs43l22_play((uint32_t *)IsocOutRdPtr, AUDIO_OUT_PACKET);

		if(IsocOutRdPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
			IsocOutRdPtr = IsocOutBuff;
		else
			IsocOutRdPtr += AUDIO_OUT_PACKET;

		if(IsocOutRdPtr == IsocOutWrPtr)
		{
			cs43l22_stop();
			PlayFlag = 0;
			IsocOutRdPtr = IsocOutBuff;
			IsocOutWrPtr = IsocOutBuff;
		}
	}
	return USBD_OK;
}

static uint8_t usbd_audio_IsoINIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t usbd_audio_IsoOUTIncomplete(void *pdev)
{
	pdev = pdev;
	return USBD_OK;
}

static uint8_t *usbd_audio_GetConfigDescriptor(uint8_t speed, uint16_t *length)
{
	speed = speed;
	*length = sizeof(usbd_audio_ConfigDescriptor);
	return usbd_audio_ConfigDescriptor;
}

static void usbd_audio_ReqSetCur(void *pdev, USB_SETUP_REQ *req)
{
	if(req->wLength)
	{
		USBD_CtlPrepareRx(pdev, AC_data, req->wLength);
		AC_bRequest = req->bRequest;
		AC_wValue = req->wValue;
		AC_wIndex = req->wIndex;
		AC_wLength = req->wLength;
	}
}

static void usbd_audio_ReqGetCur(void *pdev, USB_SETUP_REQ *req)
{
	if(HIBYTE(req->wValue) == 0x01)	//Mute
	{
		AC_data[0] = MuteFlag;
		USBD_CtlSendData(pdev, AC_data, req->wLength);
	}
	if(HIBYTE(req->wValue) == 0x02)	//Volume
	{
		AC_data[0] = LOBYTE(CurrentVolume);
		AC_data[1] = HIBYTE(CurrentVolume);
		USBD_CtlSendData(pdev, AC_data, req->wLength);
	}
}

static void usbd_audio_ReqGetMin(void *pdev, USB_SETUP_REQ *req)
{
	if(HIBYTE(req->wValue) == 0x02)	//Volume
	{
		AC_data[0] = LOBYTE(0x8000);
		AC_data[1] = HIBYTE(0x8000);
		USBD_CtlSendData(pdev, AC_data, req->wLength);
	}
}

static void usbd_audio_ReqGetMax(void *pdev, USB_SETUP_REQ *req)
{
	if(HIBYTE(req->wValue) == 0x02)	//Volume
	{
		AC_data[0] = LOBYTE(0x0000);
		AC_data[1] = HIBYTE(0x0000);
		USBD_CtlSendData(pdev, AC_data, req->wLength);
	}
}

static void usbd_audio_ReqGetRes(void *pdev, USB_SETUP_REQ *req)
{
	if(HIBYTE(req->wValue) == 0x02)	//Volume
	{
		AC_data[0] = LOBYTE(0x0147);
		AC_data[1] = HIBYTE(0x0147);
		USBD_CtlSendData(pdev, AC_data, req->wLength);
	}
}
