/*
 * lwipopts.h
 *
 *  Created on: Sep 25, 2023
 *      Author: Kai Horstmann horstmannkai@hotmai.com
 *
 *  Copyright 2023 Kai Horstmann
 *
 *  This file is published under GNU Public license (GPL) version 3.0 or later.
 */

#ifndef INC_LWIPOPTS_H_
#define INC_LWIPOPTS_H_

#include "FreeRTOSConfig.h"

/* *****************************************************************************
 * Infrastructure
 */

#define LWIP_HOSTNAME "horOV"

/*
 * NO_SYS
 */
#define 	NO_SYS   0

#define LWIP_PROVIDE_ERRNO 1

#define portTICK_RATE_MS (1000/(configTICK_RATE_HZ))

/*
 * NETIF
 */

#define 	LWIP_SINGLE_NETIF   1

#define 	LWIP_NETIF_HOSTNAME   1

#define LWIP_NUM_NETIF_CLIENT_DATA      1

/* *****************************************************************************
 * Protocols and protocol modules
 */

/*
 * LWIP_IPV4
 */

#define 	LWIP_IPV4   1

/*
 * ARP
 */
#define 	LWIP_ARP   1

/*
 * ICMP
 */

#define 	LWIP_ICMP   1

#define LWIP_BROADCAST_PING             1

#define LWIP_MULTICAST_PING             1

/*
 * DHCP
 */
#define 	LWIP_DHCP   1

#define LWIP_DHCP_DISCOVER_ADD_HOSTNAME 1

/*
 * AUTOIP
 */

#define 	LWIP_AUTOIP   1

#define 	LWIP_DHCP_AUTOIP_COOP   1

#define 	LWIP_DHCP_AUTOIP_COOP_TRIES   5

// ACD is required for autoip
#define LWIP_ACD 1
/*
 * IGMP
 */

#define 	LWIP_IGMP   1

/*
 * IPV6
 */

#define 	LWIP_IPV6   1

#define     LWIP_IPV6_DHCP6   1

/* *****************************************************************************
 * Callback-style APIs
 */

/*
 * RAW
 */

#define 	LWIP_RAW   0

/*
 * TCP
 */
#define 	LWIP_TCP   1


/*
 * UDP
 */

#define 	LWIP_UDP   1


/*
 * DNS
 */

#define 	LWIP_DNS   0

#define LWIP_DNS_SECURE 0

#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC   1

#define LWIP_DNS_SUPPORT_MDNS_QUERIES   1

/*
 * MDNS
 */

#define LWIP_MDNS_RESPONDER 1

/* *****************************************************************************
 * Thread-safe APIs
 */

/*
 * Netconn
 */

#define 	LWIP_NETCONN   1

#define 	LWIP_TCPIP_TIMEOUT   0

#define 	LWIP_NETCONN_SEM_PER_THREAD   0

#define 	LWIP_NETCONN_FULLDUPLEX   0

/*
 * TCP
 */

#define 	LWIP_SOCKET   1

#define     LWIP_POSIX_SOCKETS_IO_NAMES   0

#define     LWIP_TCP_KEEPALIVE   1
/*
 * SNMP MIB2 callbacks
 */
#define 	LWIP_MIB2_CALLBACKS   0

/*
 * Multicast
 */

#define 	LWIP_MULTICAST_TX_OPTIONS   ((LWIP_IGMP || LWIP_IPV6_MLD) && (LWIP_UDP || LWIP_RAW))

/*
 * Threading
 */

#define 	TCPIP_THREAD_NAME   "tcpip_thread"

// 2K stack got the TCPIP stack
#define 	TCPIP_THREAD_STACKSIZE   (configMINIMAL_STACK_SIZE * 16)

#define 	TCPIP_THREAD_PRIO   2

#define 	TCPIP_MBOX_SIZE   10

#define 	DEFAULT_THREAD_NAME   "lwIP"

#define 	DEFAULT_THREAD_STACKSIZE   (configMINIMAL_STACK_SIZE * 4)

#define 	DEFAULT_THREAD_PRIO   1

#define     SLIPIF_THREAD_STACKSIZE   (configMINIMAL_STACK_SIZE * 2)

#define     DEFAULT_RAW_RECVMBOX_SIZE   4

#define     DEFAULT_UDP_RECVMBOX_SIZE   4

#define     DEFAULT_TCP_RECVMBOX_SIZE   6

#define     DEFAULT_ACCEPTMBOX_SIZE   2
/*
 * Checksum
 */

/*
 * Hooks
 */

// not used.

/*
 *  Heap and memory pools
 */

// Use builtin malloc/free
#define 	MEM_LIBC_MALLOC   1

// Use malloc/free for all pbuf allocations instead of pools.
#define 	MEMP_MEM_MALLOC   1

#define 	MEM_ALIGNMENT   4

#define MEM_SIZE                        16000

/*
 * Internal memory pools
 */


/*
 * PBUF
 */


 /******************************************************************************/
/*
 * Debugging
 */


/*
 * Assertions
 */

//#define 	LWIP_NOASSERT


/*
 * Statistics
 */

#define 	LWIP_STATS   1

#define 	LWIP_STATS_DISPLAY   1

#define 	LINK_STATS   1

#define 	ETHARP_STATS   (LWIP_ARP)

#define 	IP_STATS   1

//#define 	IPFRAG_STATS   (IP_REASSEMBLY || IP_FRAG)
#define 	IPFRAG_STATS 0

#define 	ICMP_STATS   1

#define 	IGMP_STATS   (LWIP_IGMP)

#define 	UDP_STATS   (LWIP_UDP)

#define 	TCP_STATS   (LWIP_TCP)

#define 	MEM_STATS   ((MEM_LIBC_MALLOC == 0) && (MEM_USE_POOLS == 0))

#define 	MEMP_STATS   (MEMP_MEM_MALLOC == 0)

#define 	SYS_STATS   (NO_SYS == 0)

#define 	IP6_STATS   (LWIP_IPV6)

#define 	ICMP6_STATS   (LWIP_IPV6 && LWIP_ICMP6)

//#define 	IP6_FRAG_STATS   (LWIP_IPV6 && (LWIP_IPV6_FRAG || LWIP_IPV6_REASS))
#define 	IP6_FRAG_STATS 0

#define 	MLD6_STATS   (LWIP_IPV6 && LWIP_IPV6_MLD)
// #define 	MLD6_STATS   0

#define 	ND6_STATS   (LWIP_IPV6)

#define 	MIB2_STATS   0

/*
 * Debug messages
 */

#define LWIP_DEBUG 1

#define 	LWIP_DBG_MIN_LEVEL   LWIP_DBG_LEVEL_ALL

#define 	LWIP_DBG_TYPES_ON   LWIP_DBG_ON

#define 	ETHARP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	NETIF_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

// #define 	PBUF_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE
#define 	PBUF_DEBUG   LWIP_DBG_OFF

#define 	API_LIB_DEBUG   LWIP_DBG_OFF

#define 	API_MSG_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	SOCKETS_DEBUG   LWIP_DBG_OFF

#define 	ICMP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	IGMP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	INET_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	IP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	IP_REASS_DEBUG   LWIP_DBG_OFF

#define 	RAW_DEBUG   LWIP_DBG_OFF

#define 	MEM_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	MEMP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	SYS_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	TIMERS_DEBUG   LWIP_DBG_OFF

#define 	TCP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	TCP_INPUT_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	TCP_FR_DEBUG   LWIP_DBG_OFF

#define 	TCP_RTO_DEBUG   LWIP_DBG_OFF

#define 	TCP_CWND_DEBUG   LWIP_DBG_OFF

#define 	TCP_WND_DEBUG   LWIP_DBG_OFF

#define 	TCP_OUTPUT_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	TCP_RST_DEBUG   LWIP_DBG_OFF

#define 	TCP_QLEN_DEBUG   LWIP_DBG_OFF

#define 	UDP_DEBUG   LWIP_DBG_OFF

#define 	TCPIP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	SLIP_DEBUG   LWIP_DBG_OFF

#define 	DHCP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	AUTOIP_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

// #define 	ACD_DEBUG LWIP_DBG_ON | LWIP_DBG_TRACE | LWIP_DBG_STATE
#define 	ACD_DEBUG LWIP_DBG_OFF

#define 	DNS_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	IP6_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define 	DHCP6_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

#define     MDNS_DEBUG   LWIP_DBG_ON | LWIP_DBG_STATE | LWIP_DBG_TRACE

/*
 * Performance
 */

#define 	LWIP_PERF   0



#endif /* INC_LWIPOPTS_H_ */
