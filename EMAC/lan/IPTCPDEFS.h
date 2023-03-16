#ifndef _H_TCPIP_DEFS_H_
#define _H_TCPIP_DEFS_H_

// frame types (stored in Type/Length field)
#define FRAME_ARP            ((unsigned short)0x0806)              
#define FRAME_IP             ((unsigned short)0x0800)
#define SWAPB(Word)          ((unsigned short)((Word) << 8) | ((Word) >> 8))

// ------------------------------------------- DHCP DEFINITIONS -------------------------------------------

#ifdef  NOBORLAND
typedef __packed struct _DHCPFORM {
#else
typedef struct _DHCPFORM {
#endif 
	unsigned char Op;
	unsigned char HType;
	unsigned char HLen;
	unsigned char Hops;
	unsigned long XID;
	unsigned short Secs;
	unsigned short Flags;
	
	unsigned long CIAddr;
	unsigned long YIAddr;			// новый адресс клиента 
	unsigned long SIAddr;			// IP адресс сервера
	unsigned long GIAddr;
	
	unsigned char CHAddr[16];
	unsigned char SName[64];
	
	unsigned char File[128];
	// + options max size 340 bytes
	
} DHCPFORM;



#ifdef  NOBORLAND
typedef __packed struct _DHCPOPT {
#else
typedef struct _DHCPOPT {
#endif 
	unsigned char Code;
	unsigned char Len;
} DHCPOPT;






#ifdef  NOBORLAND
typedef __packed struct _DHCPOPT53 {
#else
typedef struct _DHCPOPT53 {
#endif 
	unsigned char Code;
	unsigned char Len;
	unsigned char Dat;
} DHCPOPT53;



#ifdef  NOBORLAND
typedef __packed struct _DHCPOPT54 {
#else
typedef struct _DHCPOPT54 {
#endif 
	unsigned char Code;
	unsigned char Len;
	unsigned long IP;
} DHCPOPT54;



#ifdef  NOBORLAND
typedef __packed struct _DHCPOPT51 {
#else
typedef struct _DHCPOPT51 {
#endif 
	unsigned char Code;
	unsigned char Len;
	unsigned long IP;
} DHCPOPT51;



#ifdef  NOBORLAND
typedef __packed struct _DHCPOPT1 {
#else
typedef struct _DHCPOPT1 {
#endif 
	unsigned char Code;
	unsigned char Len;
	unsigned long Mask;
} DHCPOPT1;



// ------------------------------------------ ETHERNET DEFINITIONS ------------------------------------------------

// Ethernet network layer definitions
#define ETH_DA_OFS           0                   // Destination MAC address (48 Bit)
#define ETH_SA_OFS           6                   // Source MAC address (48 Bit)
#define ETH_TYPE_OFS         12                  // Type field (16 Bit)
#define ETH_DATA_OFS         14                  // Frame Data
#define ETH_HEADER_SIZE      14


#ifdef  NOBORLAND
typedef __packed struct _macadr {
#else
typedef struct _macadr {
#endif 
	unsigned char mac[6];
} s_macadr_t;



#ifdef  NOBORLAND
typedef __packed struct _ETHMAC{
#else
typedef struct _ETHMAC{
#endif 
	unsigned char DestMAC[6];			// MAC - struct
	unsigned char SourceMAC[6];
	unsigned short EthProtocol;
} ETHMAC;



// --------------------------------------------- ARP DEFINITIONS ---------------------------------------------------

#ifdef  NOBORLAND
typedef __packed struct _EthARPStruct{
#else
typedef struct _EthARPStruct{
#endif 
	unsigned short ETH_PROTOCOL;
	unsigned short IP_PROTOCOL;
	unsigned char HardwareSize;	// 6
	unsigned char ProtocolSize;	// 4
	unsigned short OPCodeRequest;	// 1 - request, 2 - ask
	unsigned char SourceMAC[6];
	unsigned long SourceIP;
	unsigned char DestinationMAC[6];	// 0-0-0-0-0-0
	unsigned long DestinationIP;	
} EthARPStruct;


#ifdef  NOBORLAND
typedef __packed struct _EthARPFull{
#else
typedef struct _EthARPFull{
#endif 
	ETHMAC MAC_stct;
	EthARPStruct ARP_stct;
} EthARPFull;

// --------------------------------------------- IP DEFINITIONS ----------------------------------------------------

#define IP_VER_IHL_TOS_OFS   ETH_DATA_OFS + 0    // Version, Header Length, Type of Service
#define IP_TOTAL_LENGTH_OFS  ETH_DATA_OFS + 2    // IP Frame's Total Length
#define IP_IDENT_OFS         ETH_DATA_OFS + 4    // Identifying Value
#define IP_FLAGS_FRAG_OFS    ETH_DATA_OFS + 6    // Flags and Fragment Offset
#define IP_TTL_PROT_OFS      ETH_DATA_OFS + 8    // Frame's Time to Live, Protocol
#define IP_HEAD_CHKSUM_OFS   ETH_DATA_OFS + 10   // IP Frame's Header Checksum
#define IP_SOURCE_OFS        ETH_DATA_OFS + 12   // Source Address (32 Bit)
#define IP_DESTINATION_OFS   ETH_DATA_OFS + 16   // Destination Address (32 Bit)
#define IP_DATA_OFS          ETH_DATA_OFS + 20   // Frame Data (if no options)
#define IP_HEADER_SIZE       20                  // w/o options

#define IP_VER_IHL         	((unsigned short)  0x4500)              // IPv4, Header Length = 5x32 bit
#define IP_TOS_D            ((unsigned short)  0x0010)              // TOS low delay
#define IP_TOS_T            ((unsigned short)  0x0008)              // TOS high throughput
#define IP_TOS_R            ((unsigned short)  0x0004)              // TOS high reliability

#define IP_FLAG_DONTFRAG     ((unsigned short)  0x0040)              // don't fragment IP frame
#define IP_FLAG_MOREFRAG     ((unsigned short)  0x0020)              // more fragments available
#define IP_FRAGOFS_MASK      ((unsigned short)  0xFF1F)              // indicates where this fragment belongs

#define PROT_ICMP						 1
#define PROT_TCP             6                  
#define PROT_UDP             17       

#define DEFAULT_TTL          128 //64                  // Time To Live sent with packets


#ifdef  NOBORLAND
typedef __packed struct _IPStruct{
#else
typedef struct _IPStruct{
#endif 
	unsigned short IP_VER_IHL_TOS;
	unsigned short IP_TOTAL_LENGTH;
	unsigned short IP_IDENT;
	unsigned short IP_FLAGS_FRAG;
	unsigned short IP_TTL_PROT;
	unsigned short IP_HEAD_CHKSUM;
	unsigned long IP_SOURCE;
	unsigned long IP_DESTINATION;
} IPStruct;


#ifdef  NOBORLAND
typedef __packed struct _ip_t{
#else
typedef struct _ip_t{
#endif 
	__packed union {
		unsigned char c[4];
		unsigned long l;
		};
} ip_t;


// --------------------------------------------- UDP DEFINITIONS ----------------------------------------------------

#define UDP_SRCPORT_OFS (IP_DATA_OFS + 0)
#define UDP_DSTPORT_OFS (IP_DATA_OFS + 2)
#define UDP_DATALEN_OFS (IP_DATA_OFS + 4)
#define UDP_CRC_OFS     (IP_DATA_OFS + 6)
#define UDP_DATA_OFS    (IP_DATA_OFS + 8)

#define UDP_HEADER_SIZE   (2 + 2 + 2 + 2)

#ifdef  NOBORLAND
typedef __packed struct _UDPStruct{
#else
typedef struct _UDPStruct{
#endif 
	unsigned short S_Port;
	unsigned short D_Port;
	unsigned short DataLen;
	unsigned short CRC16;
} UDPStruct;



#ifdef  NOBORLAND
typedef __packed struct _UPDFrameStruct{
#else
typedef struct _UPDFrameStruct{
#endif 
	ETHMAC MAC_FRAME;
	IPStruct IP_FRAME;
	UDPStruct UDP_FRAME;
} UPDFrameStruct;

#ifdef  NOBORLAND
typedef __packed struct _UDPPoint_t {
#else
typedef struct _UDPPoint_t{
#endif 
	unsigned char MACAdress[6];
	ip_t IP;
	unsigned short UDPPort;
} UDPPoint_t;


#ifdef  NOBORLAND
typedef __packed struct _S_MACIP_t {
#else
typedef struct _S_MACIP_t{
#endif 
	ETHMAC mac;
	IPStruct ip;
} S_MACIP_t;


// ----------------------------------------------------- ICMP DEFINITIONS -----------------------------------------------------
#ifdef  NOBORLAND
typedef __packed struct _S_ICMP{
#else
typedef struct _S_ICMP{
#endif 
  unsigned char type;
  unsigned char cod;
	unsigned short crc;
	unsigned short id;
	unsigned short seqno;
} S_ICMP_t;


#ifdef  NOBORLAND
typedef __packed struct _Full_icmp_resp{
#else
typedef struct _Full_icmp_resp{
#endif 
	ETHMAC mac;
	IPStruct ip;
	S_ICMP_t icmp;
} s_icmp_resp_t;





// ------------------------------------------------------ TCP DEFINITIONS -----------------------------------------------------

enum ETCPSTATE {ETCPSTATE_NONE = 0, ETCPSTATE_OPEN = 1, ETCPSTATE_CLOSE = 2, ETCPSTATE_RX = 3};

const unsigned short C_BASE_TCP_TIMEOUT = 5000;		// 5 секунд, по стандарту 10 секунд

const unsigned char C_TCP_DATAOFS_MASK = 0x000F;
const unsigned char C_TCP_CWR_BIT = 0x80;
const unsigned char C_TCP_ECE_BIT = 0x40;
const unsigned char C_TCP_URG_BIT = 0x20;
const unsigned char C_TCP_ACK_BIT = 0x10;
const unsigned char C_TCP_PSH_BIT = 0x08;
const unsigned char C_TCP_RST_BIT = 0x04;
const unsigned char C_TCP_SYN_BIT = 0x02;
const unsigned char C_TCP_FIN_BIT = 0x01;



#ifdef  NOBORLAND
typedef __packed struct _TCP_PSEUDO_HEADER_T{
#else
typedef struct _TCP_PSEUDO_HEADER_T {
#endif 
	unsigned long ip_source;
	unsigned long ip_destination;
	unsigned short protocol;	// 6
	unsigned short size;			// sizeof(TCP_T) + data len
} TCP_PSEUDO_HEADER_T;



#ifdef  NOBORLAND
typedef __packed struct _S_TCP_T_T{
#else
typedef struct _S_TCP_T_T {
#endif 
	unsigned short src_port;
	unsigned short dst_port;
	unsigned long sn;
	unsigned long sn_ack;
	unsigned short len_flags;
	unsigned short w_size;
	unsigned short checksum;
	unsigned short urpoint;
} S_TCP_T;	



#ifdef  NOBORLAND
typedef __packed struct _TCP_FRAME_T{
#else
typedef struct _TCP_FRAME_T {
#endif 
	ETHMAC MAC;
	IPStruct IP;
	S_TCP_T TCP;
} S_TCP_MIT_T;		// MIT = Mac + Ip + Tcp




#endif
