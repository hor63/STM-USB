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

#define 	LWIP_IGMP   0

/*
 * IPV6
 */
// For the time being I am going without
#define 	LWIP_IPV6   0

#define 	IPV6_REASS_MAXAGE   60

#define 	LWIP_IPV6_SCOPES   (LWIP_IPV6 && !LWIP_SINGLE_NETIF)

#define 	LWIP_IPV6_SCOPES_DEBUG   0

#define 	LWIP_IPV6_NUM_ADDRESSES   3

#define 	LWIP_IPV6_FORWARD   0

#define 	LWIP_IPV6_FRAG   1

#define 	LWIP_IPV6_REASS   LWIP_IPV6

#define 	LWIP_IPV6_SEND_ROUTER_SOLICIT   1

#define 	LWIP_IPV6_AUTOCONFIG   LWIP_IPV6

#define 	LWIP_IPV6_ADDRESS_LIFETIMES   LWIP_IPV6_AUTOCONFIG

#define 	LWIP_IPV6_DUP_DETECT_ATTEMPTS   1

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

#define 	LWIP_COMPAT_SOCKETS   1

#define 	LWIP_POSIX_SOCKETS_IO_NAMES   0

#define 	LWIP_SOCKET_OFFSET   0

#define 	LWIP_TCP_KEEPALIVE   0

#define 	LWIP_SO_SNDTIMEO   0

#define 	LWIP_SO_RCVTIMEO   0

#define 	LWIP_SO_SNDRCVTIMEO_NONSTANDARD   0

#define 	LWIP_SO_RCVBUF   0

#define 	LWIP_SO_LINGER   0

#define 	RECV_BUFSIZE_DEFAULT   INT_MAX

#define 	LWIP_TCP_CLOSE_TIMEOUT_MS_DEFAULT   20000

#define 	SO_REUSE   0

#define 	SO_REUSE_RXTOALL   0

#define 	LWIP_FIONREAD_LINUXMODE   0

#define 	LWIP_SOCKET_SELECT   1

#define 	LWIP_SOCKET_POLL   1

/*
 * SNMP MIB2 callbacks
 */
#define 	LWIP_MIB2_CALLBACKS   0

/*
 * Multicast
 */

// #define 	LWIP_MULTICAST_TX_OPTIONS   ((LWIP_IGMP || LWIP_IPV6_MLD) && (LWIP_UDP || LWIP_RAW))
#define 	LWIP_MULTICAST_TX_OPTIONS   0

/*
 * Threading
 */

#define 	TCPIP_THREAD_NAME   "tcpip_thread"

#define 	TCPIP_THREAD_STACKSIZE   (configMINIMAL_STACK_SIZE * 4)

#define 	TCPIP_THREAD_PRIO   2

#define 	TCPIP_MBOX_SIZE   10

#define 	DEFAULT_THREAD_NAME   "lwIP"

#define 	DEFAULT_THREAD_STACKSIZE   (configMINIMAL_STACK_SIZE * 4)

#define 	DEFAULT_THREAD_PRIO   1

/*
 * Checksum
 */

#define 	LWIP_CHECKSUM_CTRL_PER_NETIF   0

#define 	CHECKSUM_GEN_IP   1

#define 	CHECKSUM_GEN_UDP   1

#define 	CHECKSUM_GEN_TCP   1

#define 	CHECKSUM_GEN_ICMP   1

#define 	CHECKSUM_GEN_ICMP6   1

#define 	CHECKSUM_CHECK_IP   1

#define 	CHECKSUM_CHECK_UDP   1

#define 	CHECKSUM_CHECK_TCP   1

#define 	CHECKSUM_CHECK_ICMP   1

#define 	CHECKSUM_CHECK_ICMP6   1

#define 	LWIP_CHECKSUM_ON_COPY   0

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

//#define 	MLD6_STATS   (LWIP_IPV6 && LWIP_IPV6_MLD)
#define 	MLD6_STATS   0

#define 	ND6_STATS   (LWIP_IPV6)

#define 	MIB2_STATS   0

/*
 * Debug messages
 */

#define 	LWIP_DBG_MIN_LEVEL   LWIP_DBG_LEVEL_ALL

#define 	LWIP_DBG_TYPES_ON   LWIP_DBG_OFF

#define 	ETHARP_DEBUG   LWIP_DBG_OFF

#define 	NETIF_DEBUG   LWIP_DBG_OFF

#define 	PBUF_DEBUG   LWIP_DBG_OFF

#define 	API_LIB_DEBUG   LWIP_DBG_OFF

#define 	API_MSG_DEBUG   LWIP_DBG_OFF

#define 	SOCKETS_DEBUG   LWIP_DBG_OFF

#define 	ICMP_DEBUG   LWIP_DBG_OFF

#define 	IGMP_DEBUG   LWIP_DBG_OFF

#define 	INET_DEBUG   LWIP_DBG_OFF

#define 	IP_DEBUG   LWIP_DBG_OFF

#define 	IP_REASS_DEBUG   LWIP_DBG_OFF

#define 	RAW_DEBUG   LWIP_DBG_OFF

#define 	MEM_DEBUG   LWIP_DBG_OFF

#define 	MEMP_DEBUG   LWIP_DBG_OFF

#define 	SYS_DEBUG   LWIP_DBG_OFF

#define 	TIMERS_DEBUG   LWIP_DBG_OFF

#define 	TCP_DEBUG   LWIP_DBG_OFF

#define 	TCP_INPUT_DEBUG   LWIP_DBG_OFF

#define 	TCP_FR_DEBUG   LWIP_DBG_OFF

#define 	TCP_RTO_DEBUG   LWIP_DBG_OFF

#define 	TCP_CWND_DEBUG   LWIP_DBG_OFF

#define 	TCP_WND_DEBUG   LWIP_DBG_OFF

#define 	TCP_OUTPUT_DEBUG   LWIP_DBG_OFF

#define 	TCP_RST_DEBUG   LWIP_DBG_OFF

#define 	TCP_QLEN_DEBUG   LWIP_DBG_OFF

#define 	UDP_DEBUG   LWIP_DBG_OFF

#define 	TCPIP_DEBUG   LWIP_DBG_OFF

#define 	SLIP_DEBUG   LWIP_DBG_OFF

#define 	DHCP_DEBUG   LWIP_DBG_OFF

#define 	AUTOIP_DEBUG   LWIP_DBG_OFF

#define 	DNS_DEBUG   LWIP_DBG_OFF

#define 	IP6_DEBUG   LWIP_DBG_OFF

#define 	DHCP6_DEBUG   LWIP_DBG_OFF

/*
 * Performance
 */

#define 	LWIP_PERF   0



#endif /* INC_LWIPOPTS_H_ */
