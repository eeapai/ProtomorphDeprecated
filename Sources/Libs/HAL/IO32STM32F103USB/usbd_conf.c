/**
******************************************************************************
* @file    USB_Device/MSC_Standalone/Src/usbd_conf.c
* @author  MCD Application Team
* @version V1.6.0
* @date    12-May-2017
* @brief   This file implements the USB Device library callbacks and MSP
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright © 2016 STMicroelectronics International N.V.
* All rights reserved.</center></h2>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted, provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of STMicroelectronics nor the names of other
*    contributors to this software may be used to endorse or promote products
*    derived from this software without specific written permission.
* 4. This software, including modifications and/or derivative works of this
*    software, must execute solely and exclusively on microcontroller or
*    microprocessor devices manufactured by or for STMicroelectronics.
* 5. Redistribution and use of this software other than as permitted under
*    this license is void and will automatically terminate your rights under
*    this license.
*
* THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
* RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
* SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------ */
#include "stm32f1xx_hal.h"
#include "usbd_core.h"
#include "usbd_io32.h"

#if defined(BOARD_LCD_CAM)
#define USB_DISCONNECT_PORT                 GPIOC
#define USB_DISCONNECT_PIN                  GPIO_PIN_15
#elif defined(BOARD_BLUE_PILL)
#define USB_DISCONNECT_PORT                 GPIOB
#define USB_DISCONNECT_PIN                  GPIO_PIN_2
#endif

PCD_HandleTypeDef hpcd;

// MCU Specific Package (MSP) callback
void HAL_PCD_MspInit(PCD_HandleTypeDef * hpcd)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
  GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


#if defined(BOARD_LCD_CAM)
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = USB_DISCONNECT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  HAL_GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStruct);
#elif defined(BOARD_BLUE_PILL)
  __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

  __HAL_RCC_USB_CLK_ENABLE();

  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

// USB Peripheral Controller Driver (PCD) callbacks; call to core
#if 1
void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef * hpcd, uint8_t state)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if (state != 0)
  {
#if defined(BOARD_LCD_CAM)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
#elif defined(BOARD_BLUE_PILL)
    HAL_GPIO_WritePin(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = USB_DISCONNECT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStruct);
#endif
  }
  else
  {
#if defined(BOARD_LCD_CAM)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
#elif defined(BOARD_BLUE_PILL)
    GPIO_InitStruct.Pin = USB_DISCONNECT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStruct);
#endif
  }
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef * hpcd)
{
  USBD_StatusTypeDef ret = USBD_OK;
  USBD_HandleTypeDef *pdev = hpcd->pData;
  USBD_SetupReqTypedef    *req = &pdev->request;
  USBD_ParseSetupRequest(req, (uint8_t *)hpcd->Setup);
  if ( USB_REQ_TYPE_VENDOR == (req->bmRequest & USB_REQ_TYPE_MASK) )
  {
    pdev->ep0_state = USBD_EP0_SETUP;
    pdev->ep0_data_len = req->wLength;

    ret = pdev->pClass->Setup(pdev, req);

    if( (req->wLength == 0) && (ret == USBD_OK) )
    {
       USBD_CtlSendStatus(pdev);
    }
    return;
  }

  USBD_LL_SetupStage((USBD_HandleTypeDef *)hpcd->pData, (uint8_t *)hpcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef * hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage((USBD_HandleTypeDef *)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef * hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage((USBD_HandleTypeDef *)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef * hpcd)
{
  USBD_LL_SOF((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef * hpcd)
{
  USBD_LL_SetSpeed((USBD_HandleTypeDef *)hpcd->pData, USBD_SPEED_FULL);
  USBD_LL_Reset((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef * hpcd)
{

}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef * hpcd)
{

}
#endif


// Core driver callbacks; call to PCD
#if 1
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef * pdev)
{
  hpcd.Instance = USB;
  hpcd.Init.dev_endpoints = 8;
  hpcd.Init.ep0_mps = PCD_EP0MPS_64;
  hpcd.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd.Init.speed = PCD_SPEED_FULL;
  hpcd.Init.low_power_enable = 0;

  hpcd.pData = pdev;
  pdev->pData = &hpcd;

  HAL_PCD_Init((PCD_HandleTypeDef *)pdev->pData);

  HAL_PCDEx_PMAConfig(pdev->pData, 0x00, PCD_SNG_BUF, 0x30);
  HAL_PCDEx_PMAConfig(pdev->pData, 0x80, PCD_SNG_BUF, 0x70);
  HAL_PCDEx_PMAConfig(pdev->pData, WINUSBCOMM_IO32EPIN_ADDR, PCD_SNG_BUF, 0x00F000B0);
//  HAL_PCDEx_PMAConfig(pdev->pData, WINUSBCOMM_EPOUT_ADDR, PCD_SNG_BUF, 0x01700130);
//  HAL_PCDEx_PMAConfig(pdev->pData, WINUSBCOMM_EPIN_ADDR, PCD_DBL_BUF, 0x00F000B0);
  HAL_PCDEx_PMAConfig(pdev->pData, WINUSBCOMM_IO32EPOUT_ADDR, PCD_DBL_BUF, 0x01700130);

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef * pdev)
{
  HAL_PCD_Start((PCD_HandleTypeDef *)pdev->pData);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  HAL_PCD_EP_Open((PCD_HandleTypeDef *)pdev->pData, ep_addr, ep_mps, ep_type);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Close((PCD_HandleTypeDef *)pdev->pData, ep_addr);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Flush((PCD_HandleTypeDef *)pdev->pData, ep_addr);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_SetStall((PCD_HandleTypeDef *)pdev->pData, ep_addr);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_ClrStall((PCD_HandleTypeDef *)pdev->pData, ep_addr);
  return USBD_OK;
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef * pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *)pdev->pData;

  if ((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef * pdev, uint8_t dev_addr)
{
  HAL_PCD_SetAddress((PCD_HandleTypeDef *)pdev->pData, dev_addr);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef * pdev, uint8_t ep_addr, uint8_t * pbuf, uint16_t size)
{
  HAL_PCD_EP_Transmit((PCD_HandleTypeDef *)pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef * pdev, uint8_t ep_addr, uint8_t * pbuf, uint16_t size)
{
  HAL_PCD_EP_Receive((PCD_HandleTypeDef *)pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}
#endif
