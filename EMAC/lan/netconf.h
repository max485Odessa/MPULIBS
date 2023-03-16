#ifndef _H_OTHER_NET_CONF_H_
#define _H_OTHER_NET_CONF_H_

//#include "stm32f2xx.h"

/* MAC Address definition *****************************************************/


#define MAC_ADDR0   0x4C			// 2B
#define MAC_ADDR1   0xCC			// A0
#define MAC_ADDR2   0x6A			// 4E
//#define MAC_ADDR3   0xBB 			// 2D
//#define MAC_ADDR4   0x4C				// 9
//#define MAC_ADDR5   0x23				// 2



typedef struct {
	unsigned char n[12];
} SSTMSERIALN;


typedef struct {
	unsigned char n[3];
} SMACNIC;


typedef struct {
	unsigned char n[3];
} SMACOUI;


 
/* Static IP Address definition ***********************************************/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   120
   
/* NETMASK definition *********************************************************/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/* Gateway Address definition *************************************************/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1

#endif


