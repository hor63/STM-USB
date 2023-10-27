/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v3.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* replace CDC-ACM includes with the CDC-RNDIS includes */
#if 0
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

/* USER CODE BEGIN Includes */
#endif

#include "usb_device.h"
#include "usbd_core.h"
// #include "usbd_desc.h"
#include "usbd_cdc_rndis.h"
#include "usbd_cdc_rndis_if.h"

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

extern void Error_Handler(void);
/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef CDC_Desc;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_Device_Init(void)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment */

	  /* Obtain the device descriptor and set bDeviceClass and bDeviceSubClass to 0.
	   * CubeMX sets the class and subclass fixed to CDC-ACM on device level. This supersedes the class and subclass definition as RNDIS
	   * in the interface descrioptor.
	   * Setting the class and subclass in the device descriptor to 0 lets the class/subclass definition in the interface descrioptor becoming
	   * effective.
	   */
	  uint16_t lenDeviceDescr = 0;
	  uint8_t * deviceDescr = CDC_Desc.GetDeviceDescriptor(USBD_SPEED_FULL,&lenDeviceDescr);
	  deviceDescr[4] = 0; /*bDeviceClass*/
	  deviceDescr[5] = 0; /*bDeviceSubClass*/

/*
 * Replace the CubeMX generated initialization sequence in the User code section "USB_DEVICE_Init_PostTreatment" below.
 * Make the generated sequence invisible to the compiler.
 * My sequence below initializes the USB device with the RNDIS descriptor and RNDIS interface fops.
 */
#if 0
  /* USER CODE END USB_Device_Init_PreTreatment */
  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Device_Init_PostTreatment */
#endif


  if (USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC_RNDIS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CDC_RNDIS_RegisterInterface(&hUsbDeviceFS, &USBD_CDC_RNDIS_fops) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE END USB_Device_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

