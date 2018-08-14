/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_gs_can.h"
//#include "main.h"

PCD_HandleTypeDef hpcd_USB;

extern USBD_HandleTypeDef USBD_Device;

void HAL_PCD_MspInit(PCD_HandleTypeDef* hpcd)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
	if(hpcd->Instance == USB_OTG_FS)
	{
		/* Configure USB FS GPIOs */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		
		/* Configure DM DP Pins */
		GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
		
		/* Configure VBUS Pin */
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
				
		/* Configure ID pin */
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
		/* Enable USB FS Clocks */
		__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
		
		/* Set USBFS Interrupt priority */
		HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
		
		/* Enable USBFS Interrupt */
		HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
		  
		if(hpcd->Init.low_power_enable == 1)
		{
			/* Enable EXTI Line 18 for USB wakeup*/
			__HAL_USB_OTG_FS_WAKEUP_EXTI_CLEAR_FLAG();
			__HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
			__HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_IT();    
			
			/* Enable USBFS Interrupt */
			HAL_NVIC_EnableIRQ(OTG_FS_IRQn);

			if(hpcd->Init.low_power_enable == 1)
			    {
			      /* Enable EXTI Line 18 for USB wakeup*/
			      __HAL_USB_OTG_FS_WAKEUP_EXTI_CLEAR_FLAG();
			      __HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
			      __HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_IT();
			      
			      /* Set EXTI Wakeup Interrupt priority*/
			      HAL_NVIC_SetPriority(OTG_FS_WKUP_IRQn, 0, 0);
			      
			      /* Enable EXTI Interrupt */
			      HAL_NVIC_EnableIRQ(OTG_FS_WKUP_IRQn);          
			    }
		}
	}
	else if(hpcd->Instance == USB_OTG_HS)
	{
		/* Configure USB HS GPIOs */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		
		/* Configure DM DP Pins */
		GPIO_InitStruct.Pin = (GPIO_PIN_14 | GPIO_PIN_15);
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF12_OTG_HS_FS;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
		
		/* Configure VBUS Pin */
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		/* Configure ID pin */
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
				
		/* Enable USB HS Clocks */
		__HAL_RCC_USB_OTG_HS_CLK_ENABLE();
		
		/* Set USBHS Interrupt priority */
		HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
		
		/* Enable USBHS Interrupt */
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
		
		if(hpcd->Init.low_power_enable == 1)
		{
			/* Enable EXTI Line 18 for USB wakeup*/
			__HAL_USB_OTG_HS_WAKEUP_EXTI_CLEAR_FLAG();
			__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
			__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_IT();
			
			/* Enable USBFS Interrupt */
			HAL_NVIC_EnableIRQ(OTG_HS_IRQn);         

			if(hpcd->Init.low_power_enable == 1)
			{
				/* Enable EXTI Line 18 for USB wakeup*/
				__HAL_USB_OTG_HS_WAKEUP_EXTI_CLEAR_FLAG();
				__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
				__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_IT();
				
				/* Set EXTI Wakeup Interrupt priority*/
				HAL_NVIC_SetPriority(OTG_HS_WKUP_IRQn, 0, 0);
				
				/* Enable EXTI Interrupt */
				HAL_NVIC_EnableIRQ(OTG_HS_WKUP_IRQn);          
			}
		}
	}
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* hpcd)
{  
  /* Disable USB Clock */
	if(hpcd->Instance == USB_OTG_FS)
	{
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
	}
	else if(hpcd->Instance == USB_OTG_HS)
	{
		__HAL_RCC_USB_OTG_HS_CLK_DISABLE();
		__HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE();
	}
  __HAL_RCC_SYSCFG_CLK_DISABLE();
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef*)hpcd->pData;
	USBD_ParseSetupRequest((USBD_SetupReqTypedef*)&pdev->request, (uint8_t*)hpcd->Setup);

	bool request_was_handled = false;

	if ((pdev->request.bmRequest & 0x1F) == USB_REQ_RECIPIENT_DEVICE ) { // device request
		request_was_handled = USBD_GS_CAN_CustomDeviceRequest(pdev, &pdev->request);
	}

	if ((pdev->request.bmRequest & 0x1F) == USB_REQ_RECIPIENT_INTERFACE ) { // interface request
		request_was_handled = USBD_GS_CAN_CustomInterfaceRequest(pdev, &pdev->request);
	}

	if (!request_was_handled) {
		USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
	}
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{ 
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
  
  /* Set USB Current Speed */
  switch(hpcd->Init.speed)
  {
  case PCD_SPEED_HIGH:
    speed = USBD_SPEED_HIGH;
    break;
    
  case PCD_SPEED_FULL:
    speed = USBD_SPEED_FULL;
    break;   
    
  default:
    speed = USBD_SPEED_FULL;
    break;
  }
  
  /* Reset Device */
  USBD_LL_Reset(hpcd->pData);  
  USBD_LL_SetSpeed(hpcd->pData, speed);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_Resume((USBD_HandleTypeDef*) hpcd->pData);
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum){  USBD_LL_IsoOUTIncomplete(hpcd->pData, epnum);}
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum){  USBD_LL_IsoINIncomplete(hpcd->pData, epnum);}
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {  USBD_LL_DevConnected(hpcd->pData);}
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd){  USBD_LL_DevDisconnected(hpcd->pData);}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{ 
	/* Init USB_IP */
	/* Link The driver to the stack */
	hpcd_USB.pData = pdev;
	pdev->pData = &hpcd_USB;
#ifdef USE_USBD_FS
	hpcd_USB.Instance = USB_OTG_FS;
	hpcd_USB.Init.dev_endpoints = 4;
	hpcd_USB.Init.speed = PCD_SPEED_FULL;
	hpcd_USB.Init.ep0_mps = DEP0CTL_MPS_64;
	hpcd_USB.Init.dma_enable = 0;
	hpcd_USB.Init.use_dedicated_ep1 = 0;
	hpcd_USB.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB.Init.low_power_enable = 0;
	hpcd_USB.Init.lpm_enable = 0;
	hpcd_USB.Init.Sof_enable = 1;
	hpcd_USB.Init.vbus_sensing_enable = 0;
	HAL_PCD_Init(&hpcd_USB);
#elif USE_USBD_HS
	hpcd_USB.Instance = USB_OTG_HS;
	hpcd_USB.Init.dev_endpoints = 4;
	hpcd_USB.Init.speed = PCD_SPEED_FULL;
	hpcd_USB.Init.ep0_mps = DEP0CTL_MPS_64;
	hpcd_USB.Init.dma_enable = 0;
	hpcd_USB.Init.use_dedicated_ep1 = 0;
	hpcd_USB.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB.Init.low_power_enable = 0;
	hpcd_USB.Init.lpm_enable = 0;
	hpcd_USB.Init.Sof_enable = 1;
	hpcd_USB.Init.vbus_sensing_enable = 0;
	HAL_PCD_Init(&hpcd_USB);
#else
#error Use FS or HS
#endif
	
	/* total is what? */
	HAL_PCDEx_SetRxFiFo(&hpcd_USB, 0x80); // all EPs
	HAL_PCDEx_SetTxFiFo(&hpcd_USB, 0, 0x40); // setup
	HAL_PCDEx_SetTxFiFo(&hpcd_USB, 1, 0x40); // GSUSB_ENDPOINT_IN
	HAL_PCDEx_SetTxFiFo(&hpcd_USB, 2, 0x80); // GSUSB_ENDPOINT_OUT
	
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_DeInit((PCD_HandleTypeDef*)pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_Start((PCD_HandleTypeDef*)pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
	HAL_PCD_Stop((PCD_HandleTypeDef*) pdev->pData);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	HAL_PCD_EP_Open((PCD_HandleTypeDef*) pdev->pData, ep_addr, ep_mps, ep_type);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_Close((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_Flush((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_SetStall((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_PCD_EP_ClrStall((PCD_HandleTypeDef*) pdev->pData, ep_addr);
	return USBD_OK;
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;
  
  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
	HAL_PCD_SetAddress((PCD_HandleTypeDef*) pdev->pData, dev_addr);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_PCD_EP_Transmit((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_PCD_EP_Receive((PCD_HandleTypeDef*) pdev->pData, ep_addr, pbuf, size);
	return USBD_OK;
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
