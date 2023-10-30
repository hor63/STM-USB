/**
  ******************************************************************************
  * @file    Src/usbd_cdc_ecm_if_template.c
  * @author  MCD Application Team
  * @brief   Source file for USBD CDC_ECM interface
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

#include "usbd_cdc_ecm_if.h"
/*

  Include here  LwIP files if used

*/
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "lwip/ethip6.h"
#include "lwip/dhcp.h"
#include "lwip/dhcp6.h"
#include "lwip/autoip.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "lwip/apps/mdns.h"
#include "lwip/igmp.h"
#include "lwip/apps/netbiosns.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "usbd_cdc_ecm_if.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	struct pbuf *p;
} tInputQueueItem;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* USB handler declaration */
// extern USBD_HandleTypeDef  hUsbDeviceFS ;

/* Received Data over USB are stored in this buffer */
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* ( __ICCARM__ ) */
__ALIGN_BEGIN static uint8_t UserRxBuffer[CDC_ECM_ETH_MAX_SEGSZE + 100]__ALIGN_END;

/* Transmitted Data over CDC_ECM (CDC_ECM interface) are stored in this buffer */
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* ( __ICCARM__ ) */
__ALIGN_BEGIN  static uint8_t UserTxBuffer[CDC_ECM_ETH_MAX_SEGSZE + 100]__ALIGN_END;

static uint8_t CDC_ECMInitialized = 0U;

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

/* The netif representing the ECM interface */
static struct netif ecmIF;

#if LWIP_DHCP
static struct dhcp dhcpECM;
#endif
#if LWIP_AUTOIP
static struct autoip autoipECM;
#endif
#if LWIP_IPV6_DHCP6
static struct dhcp6 dhcp6ECM;
#endif

/* USB handler declaration */
extern USBD_HandleTypeDef  hUsbDeviceFS;

/* Private function prototypes -----------------------------------------------*/

static void staticFreeRTOSInit();
static void tcpip_init_done(void *arg);

static int8_t CDC_ECM_Itf_Init(void);
static int8_t CDC_ECM_Itf_DeInit(void);
static int8_t CDC_ECM_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_ECM_Itf_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_ECM_Itf_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);
static int8_t CDC_ECM_Itf_Process(USBD_HandleTypeDef *pdev);

USBD_CDC_ECM_ItfTypeDef USBD_CDC_ECM_fops =
{
  CDC_ECM_Itf_Init,
  CDC_ECM_Itf_DeInit,
  CDC_ECM_Itf_Control,
  CDC_ECM_Itf_Receive,
  CDC_ECM_Itf_TransmitCplt,
  CDC_ECM_Itf_Process,
  (uint8_t *)CDC_ECM_MAC_STR_DESC,
};

/* Private functions ---------------------------------------------------------*/


static  err_t CDC_ECM_Itf_SendEthFrame (struct netif *netif, struct pbuf *p) {

	USBD_CDC_ECM_HandleTypeDef *hcdc = (USBD_CDC_ECM_HandleTypeDef *)hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];
	struct pbuf *pBufChain;
	uint32_t bufPos = 0U;
	struct eth_hdr *ethhdr;
	int i = 0;

	while (hcdc->TxState != 0) {
		/* Wait one tick, i.e. the shortest possible delay (no matter how long that is) */
		vTaskDelay(1);
		i++;
		if (i >= 50) {
			LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE,("CDC_ECM_Itf_SendEthFrame: Waited 50ms, timeout. Send anyway.\n"));
			hcdc->TxState = 0;
		}
	}

	for (pBufChain = p; pBufChain != NULL;pBufChain=pBufChain->next) {

		/*
		 * If the length of the pbuf fragments exceeds my send buffer flush it in between.
		 */
		if ((bufPos + pBufChain->len) > sizeof(UserTxBuffer)) {
			//printf ("Send partial frame len %lu= \n",bufPos);
			USBD_CDC_ECM_SetTxBuffer(&hUsbDeviceFS,UserTxBuffer,bufPos);
			USBD_CDC_ECM_TransmitPacket(&hUsbDeviceFS);

			while (hcdc->TxState != 0) {
				/* Wait one tick, i.e. the shortest possible delay (no matter how long that is) */
				vTaskDelay(1);
			}
			bufPos = 0;
		}
		//printf ("Copy pbuf len= %lu \n",(uint32_t)pBufChain->len);
		memcpy (&UserTxBuffer[bufPos],pBufChain->payload,pBufChain->len);
		bufPos += pBufChain->len;
	}
	LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE,("CDC_ECM_Itf_SendEthFrame: Send frame len= %lu \n",bufPos));
    ethhdr = (struct eth_hdr *)p->payload;
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE,
	              ("CDC_ECM_Itf_SendEthFrame: dest:%"X16_F":%"X16_F":%"X16_F":%"X16_F":%"X16_F":%"X16_F", src:%"X16_F":%"X16_F":%"X16_F":%"X16_F":%"X16_F":%"X16_F", type:%"X16_F"\n",
	               (unsigned short)ethhdr->dest.addr[0], (unsigned short)ethhdr->dest.addr[1], (unsigned short)ethhdr->dest.addr[2],
	               (unsigned short)ethhdr->dest.addr[3], (unsigned short)ethhdr->dest.addr[4], (unsigned short)ethhdr->dest.addr[5],
	               (unsigned short)ethhdr->src.addr[0],  (unsigned short)ethhdr->src.addr[1],  (unsigned short)ethhdr->src.addr[2],
	               (unsigned short)ethhdr->src.addr[3],  (unsigned short)ethhdr->src.addr[4],  (unsigned short)ethhdr->src.addr[5],
	               lwip_htons(ethhdr->type)));

	USBD_CDC_ECM_SetTxBuffer(&hUsbDeviceFS,UserTxBuffer,bufPos);
	USBD_CDC_ECM_TransmitPacket(&hUsbDeviceFS);

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
  netif->linkoutput = CDC_ECM_Itf_SendEthFrame;
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif

  /* initialize the hardware */
  /* set MAC hardware address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* Set MAC hardware address */
  netif->hwaddr[0] =  (u8_t)(CDC_ECM_MAC_ADDR0 + 0x16) & 0xffU;
  netif->hwaddr[1] =  (u8_t)(CDC_ECM_MAC_ADDR1 + 0x15) & 0xffU;
  netif->hwaddr[2] =  (u8_t)(CDC_ECM_MAC_ADDR2 + 0x14) & 0xffU;
  netif->hwaddr[3] =  (u8_t)(CDC_ECM_MAC_ADDR3 + 0x13) & 0xffU;
  netif->hwaddr[4] =  (u8_t)(CDC_ECM_MAC_ADDR4 + 0x12) & 0xffU;
  netif->hwaddr[5] =  (u8_t)(CDC_ECM_MAC_ADDR5 + 0x11) & 0xffU;

  /* maximum transfer unit */
  netif->mtu = CDC_ECM_ETH_MTU;
#if LWIP_IPV6
  netif->mtu6 = CDC_ECM_ETH_MTU;
#endif

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHERNET
#if LWIP_ARP
		  | NETIF_FLAG_ETHARP
#endif
#if LWIP_IPV6_MLD
		  | NETIF_FLAG_MLD6
#endif
#if LWIP_IGMP
		  | NETIF_FLAG_IGMP
#endif
		  ;

  return (err_t)ERR_OK;
}

/**
 * @brief CDC_ECM_Static_Init
 * 		Performs static initialization of this module before FreeeRTOS tasks start running.
 */
void CDC_ECM_StaticInit () {
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

void CDC_ECM_LWIPInputLoop () {

	static tInputQueueItem queueItem;
	if (inputLoopQueue != NULL) {
		for (;;) {
			if (xQueueReceive(
				inputLoopQueue,
				&queueItem,
				portMAX_DELAY
					) == pdTRUE) {

				/* If the pbuf is NULL this is the indicator
				 * that this queue message was not a received message,
				 * but instead the initialization of LWIP
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
					// printf("CDC_ECM_Itf_Receive: received %lu bytes from host\n",(uint32_t)queueItem.p->len);

					tcpip_input(queueItem.p,&ecmIF);
				}
			}

		}
	}

}

static void tcpip_init_done(void *arg) {

	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gateway;

#if defined FIX_IP_ADDRESS
	ip4addr_aton(FIX_IP_ADDRESS, &ipaddr);
#else
	ipaddr.addr = IPADDR_ANY;
#endif
#if defined FIX_IP_NETMASK
	ip4addr_aton(FIX_IP_NETMASK, &netmask);
#else
	netmask.addr = IPADDR_ANY;
#endif
#if defined FIX_IP_GATEWAY
	ip4addr_aton(FIX_IP_GATEWAY, &gateway);
#else
	gateway.addr = IPADDR_ANY;
#endif

	memset (&ecmIF,0,sizeof(ecmIF));
	netif_init();
	netif_add (&ecmIF,&ipaddr,&netmask,&gateway, NULL, ethernetif_init, ethernet_input);

	netif_set_default(&ecmIF);
	netif_set_hostname(&ecmIF,LWIP_HOSTNAME);

	netif_set_link_up(&ecmIF);
	netif_set_up(&ecmIF);

#if LWIP_IGMP
	igmp_init();
	igmp_start(&ecmIF);
#endif

	/* Startup DHCP and DHCP6 */
#if LWIP_DHCP
	dhcp_set_struct(&ecmIF,&dhcpECM);
#endif
#if LWIP_AUTOIP
	autoip_set_struct(&ecmIF, &autoipECM);
#endif
#if LWIP_IPV6_DHCP6
	dhcp6_set_struct(&ecmIF,&dhcp6ECM);
#endif

#if LWIP_DHCP
	dhcp_start(&ecmIF);
#endif
#if LWIP_AUTOIP
	autoip_start(&ecmIF);
#endif
#if LWIP_IPV6_DHCP6
	dhcp6_enable_stateless(&ecmIF);
#endif

#if LWIP_MDNS_RESPONDER
	mdns_resp_init();
	mdns_resp_add_netif(&ecmIF, LWIP_HOSTNAME);
#endif

#if LWIP_DNS
	dns_init();
#endif

#if LWIP_NETBIOS_RESPOND_NAME_QUERY
	netbiosns_init();
	netbiosns_set_name(ecmIF.hostname);
#endif

}

/**
  * @brief  CDC_ECM_Itf_Init
  *         Initializes the CDC_ECM media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_Init(void)
{
	tInputQueueItem queueItem;
	BaseType_t higherPrioTaskHasWoken = pdFALSE;

  if (CDC_ECMInitialized == 0U)
  {
    /*
      Initialize the TCP/IP stack here
    */
	/* A NULL pbuf is the indicator for the input loop
	 * to initialize LWIP.
	 */
	queueItem.p = NULL;
	xQueueSendToBackFromISR(inputLoopQueue,&queueItem,&higherPrioTaskHasWoken);

    CDC_ECMInitialized = 1U;
  }

  /* Set Application Buffers */
#ifdef USE_USBD_COMPOSITE
  (void)USBD_CDC_ECM_SetTxBuffer(&hUsbDeviceFS, UserTxBuffer, 0U, 0U);
#else
  (void)USBD_CDC_ECM_SetTxBuffer(&hUsbDeviceFS, UserTxBuffer, 0U);
#endif /* USE_USBD_COMPOSITE */
  (void)USBD_CDC_ECM_SetRxBuffer(&hUsbDeviceFS, UserRxBuffer);

  portYIELD_FROM_ISR (higherPrioTaskHasWoken);

  return (0);
}

/**
  * @brief  CDC_ECM_Itf_DeInit
  *         DeInitializes the CDC_ECM media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_DeInit(void)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *) \
                                             (hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId]);
#else
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *)(hUsbDeviceFS.pClassData);
#endif /* USE_USBD_COMPOSITE */

  /* Notify application layer that link is down */
  hcdc_cdc_ecm->LinkStatus = 0U;

  return (0);
}

/**
  * @brief  CDC_ECM_Itf_Control
  *         Manage the CDC_ECM class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *) \
                                             (hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId]);
#else
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *)(hUsbDeviceFS.pClassData);
#endif /* USE_USBD_COMPOSITE */

  switch (cmd)
  {
    case CDC_ECM_SEND_ENCAPSULATED_COMMAND:
      /* Add your code here */
      break;

    case CDC_ECM_GET_ENCAPSULATED_RESPONSE:
      /* Add your code here */
      break;

    case CDC_ECM_SET_ETH_MULTICAST_FILTERS:
      /* Add your code here */
      break;

    case CDC_ECM_SET_ETH_PWRM_PATTERN_FILTER:
      /* Add your code here */
      break;

    case CDC_ECM_GET_ETH_PWRM_PATTERN_FILTER:
      /* Add your code here */
      break;

    case CDC_ECM_SET_ETH_PACKET_FILTER:
      /* Check if this is the first time we enter */
      if (hcdc_cdc_ecm->LinkStatus == 0U)
      {
        /*
          Setup the Link up at TCP/IP level
        */
        hcdc_cdc_ecm->LinkStatus = 1U;

        /* Modification for MacOS which doesn't send SetInterface before receiving INs */
        if (hcdc_cdc_ecm->NotificationStatus == 0U)
        {
          /* Send notification: NETWORK_CONNECTION Event */
          (void)USBD_CDC_ECM_SendNotification(&hUsbDeviceFS, NETWORK_CONNECTION,
                                              CDC_ECM_NET_CONNECTED, NULL);

          /* Prepare for sending Connection Speed Change notification */
          hcdc_cdc_ecm->NotificationStatus = 1U;
        }
      }
      /* Add your code here */
      break;

    case CDC_ECM_GET_ETH_STATISTIC:
      /* Add your code here */
      break;

    default:
      break;
  }
  UNUSED(length);
  UNUSED(pbuf);

  return (0);
}

/**
  * @brief  CDC_ECM_Itf_Receive
  *         Data received over USB OUT endpoint are sent over CDC_ECM interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_Receive(uint8_t *Buf, uint32_t *Len)
{
  tInputQueueItem queueItem;
  BaseType_t higherPrioTaskHasWoken = pdFALSE;

  /* Get the CDC_ECM handler pointer */
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *) \
                                             (hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId]);
#else
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *)(hUsbDeviceFS.pClassData);
#endif /* USE_USBD_COMPOSITE */

  /* Call Eth buffer processing */
  hcdc_cdc_ecm->RxState = 1U;

  queueItem.p = pbuf_alloc(PBUF_RAW, *Len, PBUF_RAM);
  if (queueItem.p) {
	  memcpy (queueItem.p->payload,Buf,*Len);
	  xQueueSendToBackFromISR(inputLoopQueue,&queueItem,&higherPrioTaskHasWoken);
  }

  /* Reset the Received buffer length to zero for next transfer */
  hcdc_cdc_ecm->RxLength = 0;
  hcdc_cdc_ecm->RxState = 0;

    /* Reset the Rx buffer pointer to origin */
  (void) USBD_CDC_ECM_SetRxBuffer(&hUsbDeviceFS , UserRxBuffer);
  /* Prepare Out endpoint to receive next packet in current/new frame */

  USBD_CDC_ECM_ReceivePacket (&hUsbDeviceFS );

  portYIELD_FROM_ISR (higherPrioTaskHasWoken);

  return (USBD_OK);
}

/**
  * @brief  CDC_ECM_Itf_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);

  return (0);
}

/**
  * @brief  CDC_ECM_Itf_Process
  *         Data received over USB OUT endpoint are sent over CDC_ECM interface
  *         through this function.
  * @param  pdef: pointer to the USB Device Handle
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_ECM_Itf_Process(USBD_HandleTypeDef *pdev)
{
  /* Get the CDC_ECM handler pointer */
#ifdef USE_USBD_COMPOSITE
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *)(pdev->pClassDataCmsit[pdev->classId]);
#else
  USBD_CDC_ECM_HandleTypeDef *hcdc_cdc_ecm = (USBD_CDC_ECM_HandleTypeDef *)(pdev->pClassData);
#endif /* USE_USBD_COMPOSITE */

  if (hcdc_cdc_ecm == NULL)
  {
    return (-1);
  }

  if (hcdc_cdc_ecm->LinkStatus != 0U)
  {
    /*
      Read a received packet from the Ethernet buffers and send it
      to the lwIP for handling
      Call here the TCP/IP background tasks.
    */
  }

  return (0);
}

