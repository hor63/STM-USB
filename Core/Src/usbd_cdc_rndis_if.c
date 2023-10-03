/**
  ******************************************************************************
  * @file    usbd_cdc_rndis_if_template.c
  * @author  MCD Application Team
  * @brief   Source file for USBD CDC_RNDIS interface template
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* Include TCP/IP stack header files */

#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "lwip/ethip6.h"
#include "lwip/dhcp.h"
#include "lwip/dhcp6.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "usbd_cdc_rndis_if.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	struct pbuf *p;
} tInputQueueItem;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* USB handler declaration */
extern USBD_HandleTypeDef  hUsbDeviceFS ;

/* Received Data over USB are stored in this buffer */
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* __ICCARM__ */
__ALIGN_BEGIN static uint8_t UserRxBuffer[CDC_RNDIS_ETH_MAX_SEGSZE + 100] __ALIGN_END;

/* Transmitted Data over CDC_RNDIS (CDC_RNDIS interface) are stored in this buffer */
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* __ICCARM__ */
__ALIGN_BEGIN static uint8_t UserTxBuffer[CDC_RNDIS_ETH_MAX_SEGSZE + 100] __ALIGN_END;

static uint8_t CDC_RNDISInitialized = 0U;

/*
 * FreeRTOS stuff
 */

/* Static input loop queue */
#define INPUT_QUEUE_LEN 4U
static tInputQueueItem inputLoopQueueStorageBuf[INPUT_QUEUE_LEN];
static StaticQueue_t inputLoopQueueBuf;
static QueueHandle_t inputLoopQueue = NULL;

/*
 * LWIP stuff
 */

/* The netif representing the RNDIS interface */
static struct netif rndisIF;

static struct dhcp dhcpRNDIS;
static struct dhcp6 dhcp6RNDIS;

/* Private function prototypes -----------------------------------------------*/

static void staticFreeRTOSInit();
static void tcpip_init_done(void *arg);

static int8_t CDC_RNDIS_Itf_Init(void);
static int8_t CDC_RNDIS_Itf_DeInit(void);
static int8_t CDC_RNDIS_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_RNDIS_Itf_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_RNDIS_Itf_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);
static int8_t CDC_RNDIS_Itf_Process(USBD_HandleTypeDef *pdev);

USBD_CDC_RNDIS_ItfTypeDef USBD_CDC_RNDIS_fops =
{
  CDC_RNDIS_Itf_Init,
  CDC_RNDIS_Itf_DeInit,
  CDC_RNDIS_Itf_Control,
  CDC_RNDIS_Itf_Receive,
  CDC_RNDIS_Itf_TransmitCplt,
  CDC_RNDIS_Itf_Process,
  (uint8_t *)CDC_RNDIS_MAC_STR_DESC,
};

/* Private functions ---------------------------------------------------------*/


static  err_t CDC_RNDIS_Itf_SendEthFrame (struct netif *netif, struct pbuf *p) {

	USBD_CDC_RNDIS_HandleTypeDef *hcdc = (USBD_CDC_RNDIS_HandleTypeDef *)hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];
	struct pbuf *pBufChain;
	uint32_t bufPos = 0;

	while (hcdc->TxState != 0) {
		/* Wait one tick, i.e. the shortest possible delay (no matter how long that is) */
		vTaskDelay(1);
	}

	for (pBufChain = p; pBufChain != NULL;pBufChain=pBufChain->next) {

		/*
		 * If the length of the pbuf fragments exceeds my send buffer flush it in between.
		 */
		if ((bufPos + pBufChain->len) > sizeof(UserTxBuffer)) {
			printf ("Send partial frame len %lu= \n",bufPos);
			USBD_CDC_RNDIS_SetTxBuffer(&hUsbDeviceFS,UserTxBuffer,bufPos);
			USBD_CDC_RNDIS_TransmitPacket(&hUsbDeviceFS);

			while (hcdc->TxState != 0) {
				/* Wait one tick, i.e. the shortest possible delay (no matter how long that is) */
				vTaskDelay(1);
			}
			bufPos = 0;
		}
		printf ("Copy pbuf len %lu= \n",(uint32_t)pBufChain->len);
		memcpy (&UserTxBuffer[bufPos],pBufChain->payload,pBufChain->len);
		bufPos += pBufChain->len;
	}
	printf ("Send frame len %lu= \n",bufPos);
	USBD_CDC_RNDIS_SetTxBuffer(&hUsbDeviceFS,UserTxBuffer,bufPos);
	USBD_CDC_RNDIS_TransmitPacket(&hUsbDeviceFS);

	return (err_t)ERR_OK;
}

static err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

  netif->name[0] = 'O';
  netif->name[1] = 'V';

  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = CDC_RNDIS_Itf_SendEthFrame;
  netif->output_ip6 = ethip6_output;

  /* initialize the hardware */
  /* set MAC hardware address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* Set MAC hardware address */
  netif->hwaddr[0] =  CDC_RNDIS_MAC_ADDR0;
  netif->hwaddr[1] =  CDC_RNDIS_MAC_ADDR1;
  netif->hwaddr[2] =  CDC_RNDIS_MAC_ADDR2;
  netif->hwaddr[3] =  CDC_RNDIS_MAC_ADDR3;
  netif->hwaddr[4] =  CDC_RNDIS_MAC_ADDR4;
  netif->hwaddr[5] =  CDC_RNDIS_MAC_ADDR5;

  /* maximum transfer unit */
  netif->mtu = CDC_RNDIS_ETH_MTU;
  netif->mtu6 = CDC_RNDIS_ETH_MTU;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_MLD6;

  return (err_t)ERR_OK;
}

/**
 * @brief CDC_RNDIS_Static_Init
 * 		Performs static initialization of this module before FreeeRTOS tasks start running.
 */
void CDC_RNDIS_StaticInit () {
	/* Create static FreeRTOS objects at the very start of the program */
	staticFreeRTOSInit();
}

static void staticFreeRTOSInit() {

	if (inputLoopQueue == NULL) {
		inputLoopQueue = xQueueCreateStatic (
				INPUT_QUEUE_LEN,
				sizeof(tInputQueueItem),
				(uint8_t *)inputLoopQueueStorageBuf,
				&inputLoopQueueBuf
				);
	}

}

void CDC_RNDIS_LWIPInputLoop () {

	static tInputQueueItem queueItem;
	if (inputLoopQueue != NULL) {
		for (;;) {
			if (xQueueReceive(
				inputLoopQueue,
				&queueItem,
				portMAX_DELAY
					) == pdTRUE) {

				/* If the pbuf is NULL this is the indicator
				 * that this queue message was not a receviced message,
				 * but instead the initialialization of LWIP
				 * is being triggered.
				 * tcpip_init must NOT be called from an ISR.
				 * Therefore I am executing it from here.
				 */
				if (queueItem.p == NULL) {
				    /*
				      Initialize the LwIP stack
				    */
					tcpip_init(tcpip_init_done,NULL);
				} else {
				/* Throw the pbuf to the LWIP task */
					tcpip_input(queueItem.p,&rndisIF);
				}
			}

		}
	}

}

static void tcpip_init_done(void *arg) {

	memset (&rndisIF,0,sizeof(rndisIF));
	netif_init();
	netif_add_noaddr(&rndisIF, NULL, ethernetif_init, ethernet_input);

	netif_set_default(&rndisIF);
	netif_set_hostname(&rndisIF,"horOV");
	netif_set_link_up(&rndisIF);
	netif_set_up(&rndisIF);

	/* Startup DHCP and DHCP6 */
	dhcp_set_struct(&rndisIF,&dhcpRNDIS);
	dhcp6_set_struct(&rndisIF,&dhcp6RNDIS);

	dhcp_start(&rndisIF);
	dhcp6_enable_stateless(&rndisIF);

}

/**
  * @brief  CDC_RNDIS_Itf_Init
  *         Initializes the CDC_RNDIS media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_Init(void)
{
	tInputQueueItem queueItem;
	BaseType_t higherPrioTaskHasWoken = pdFALSE;

  if (CDC_RNDISInitialized == 0U)
  {

	  /* A NULL pbuf is the indicator for the input loop
	   * to initialize LWIP.
	   */
	  queueItem.p = NULL;
	  xQueueSendToBackFromISR(inputLoopQueue,&queueItem,&higherPrioTaskHasWoken);

	  CDC_RNDISInitialized = 1U;
  }

  /* Set Application Buffers */
#ifdef USE_USBD_COMPOSITE
  (void)USBD_CDC_RNDIS_SetTxBuffer(&hUsbDeviceFS , UserTxBuffer, 0U, 0U);
#else
  (void)USBD_CDC_RNDIS_SetTxBuffer(&hUsbDeviceFS , UserTxBuffer, 0U);
#endif /* USE_USBD_COMPOSITE */
  (void)USBD_CDC_RNDIS_SetRxBuffer(&hUsbDeviceFS , UserRxBuffer);

  portYIELD_FROM_ISR (higherPrioTaskHasWoken);

  return (0);
}

/**
  * @brief  CDC_RNDIS_Itf_DeInit
  *         DeInitializes the CDC_RNDIS media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_DeInit(void)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *) \
                                                 (hUsbDeviceFS .pClassDataCmsit[hUsbDeviceFS .classId]);
#else
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *)(hUsbDeviceFS .pClassData);
#endif /* USE_USBD_COMPOSITE */

  /*
     Add your code here
  */

  /* Notify application layer that link is down */
  hcdc_cdc_rndis->LinkStatus = 0U;

  return (0);
}

/**
  * @brief  CDC_RNDIS_Itf_Control
  *         Manage the CDC_RNDIS class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *) \
                                                 (hUsbDeviceFS .pClassDataCmsit[hUsbDeviceFS .classId]);
#else
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *)(hUsbDeviceFS .pClassData);
#endif /* USE_USBD_COMPOSITE */

  switch (cmd)
  {
    case CDC_RNDIS_SEND_ENCAPSULATED_COMMAND:
      printf ("CDC_RNDIS_Itf_Control cmd = %d = CDC_RNDIS_SEND_ENCAPSULATED_COMMAND, len=%d\n",(int)cmd, (int)length);
      /* Add your code here */
      break;

    case CDC_RNDIS_GET_ENCAPSULATED_RESPONSE:
      printf ("CDC_RNDIS_Itf_Control cmd = %d = CDC_RNDIS_GET_ENCAPSULATED_RESPONSE, len=%d\n",(int)cmd, (int)length);
      /* Check if this is the first time we enter */
      if (hcdc_cdc_rndis->LinkStatus == 0U)
      {
        /* Setup the Link up at TCP/IP stack level */
        hcdc_cdc_rndis->LinkStatus = 1U;
        printf ("  Set link status up\n");
        /*
          Add your code here
        */
      }
      /* Add your code here */
      break;

    default:
      /* Add your code here */
      printf ("CDC_RNDIS_Itf_Control unknown cmd = %d , len=%d\n",(int)cmd, (int)length);
      break;
  }

  UNUSED(length);
  UNUSED(pbuf);

  return (0);
}

/**
  * @brief  CDC_RNDIS_Itf_Receive
  *         Data received over USB OUT endpoint are sent over CDC_RNDIS interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_Receive(uint8_t *Buf, uint32_t *Len)
{
	tInputQueueItem queueItem;
	BaseType_t higherPrioTaskHasWoken = pdFALSE;

  /* Get the CDC_RNDIS handler pointer */
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *) \
                                                 (hUsbDeviceFS .pClassDataCmsit[hUsbDeviceFS .classId]);
#else
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *)(hUsbDeviceFS .pClassData);
#endif /* USE_USBD_COMPOSITE */

  /* Call Eth buffer processing */
  hcdc_cdc_rndis->RxState = 1U;

  printf("CDC_RNDIS_Itf_Receive: received %lu bytes from host\n",*Len);

  queueItem.p = pbuf_alloc(PBUF_RAW, *Len, PBUF_RAM);
  if (queueItem.p) {
	  memcpy (queueItem.p->payload,Buf,*Len);
	  xQueueSendToBackFromISR(inputLoopQueue,&queueItem,&higherPrioTaskHasWoken);
  }

  /* Reset the Received buffer length to zero for next transfer */
  hcdc_cdc_rndis->RxLength = 0;
  hcdc_cdc_rndis->RxState = 0;

    /* Reset the Rx buffer pointer to origin */
  (void) USBD_CDC_RNDIS_SetRxBuffer(&hUsbDeviceFS , UserRxBuffer);
  /* Prepare Out endpoint to receive next packet in current/new frame */

  USBD_CDC_RNDIS_ReceivePacket (&hUsbDeviceFS );
  /* USBD_CDC_RNDIS_SetRxBuffer replaces this internal call
   *
   * (void) USBD_LL_PrepareReceive(&hUsbDeviceFS ,
   *                        CDC_RNDIS_OUT_EP,
   *                        (uint8_t*)(hcdc_cdc_rndis->RxBuffer),
   *                        (uint16_t)hcdc_cdc_rndis->MaxPcktLen);
  */

  portYIELD_FROM_ISR (higherPrioTaskHasWoken);

  return (USBD_OK);
}

/**
  * @brief  CDC_RNDIS_Itf_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @param  epnum: EP number
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);

  return (0);
}

/**
  * @brief  CDC_RNDIS_Itf_Process
  *         Data received over USB OUT endpoint are sent over CDC_RNDIS interface
  *         through this function.
  * @param  pdef: pointer to the USB Device Handle
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_RNDIS_Itf_Process(USBD_HandleTypeDef *pdev)
{
  /* Get the CDC_RNDIS handler pointer */
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *)(pdev->pClassDataCmsit[pdev->classId]);
#else
  USBD_CDC_RNDIS_HandleTypeDef *hcdc_cdc_rndis = (USBD_CDC_RNDIS_HandleTypeDef *)(pdev->pClassData);
#endif /* USE_USBD_COMPOSITE */

  if (hcdc_cdc_rndis == NULL)
  {
    return (-1);
  }

  if (hcdc_cdc_rndis->LinkStatus != 0U)
  {
    /*
       Add your code here
       Read a received packet from the Ethernet buffers and send it
       to the lwIP for handling
    */

	  printf("CDC_RNDIS_Itf_Process: received %lu bytes from host\n",hcdc_cdc_rndis->RxLength);

	  /* Reset the Received buffer length to zero for next transfer */
	  hcdc_cdc_rndis->RxLength = 0;
	  hcdc_cdc_rndis->RxState = 0;

	    /* Reset the Rx buffer pointer to origin */
	  (void) USBD_CDC_RNDIS_SetRxBuffer(&hUsbDeviceFS , UserRxBuffer);
	  /* Prepare Out endpoint to receive next packet in current/new frame */
	  (void) USBD_LL_PrepareReceive(&hUsbDeviceFS ,
	                         CDC_RNDIS_OUT_EP,
	                         (uint8_t*)(hcdc_cdc_rndis->RxBuffer),
	                         (uint16_t)hcdc_cdc_rndis->MaxPcktLen);



  }

  return (0);
}

