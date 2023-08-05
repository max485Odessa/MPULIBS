#ifndef __H_CORE_TYPES_DEFINE_H__
#define __H_CORE_TYPES_DEFINE_H__

typedef enum {EKEY_LEFT = 0, EKEY_MIDL = 1, EKEY_RIGHT = 2, EKEY_MAXCOUNTKEY = 3} EKEYCOFE;
typedef unsigned short (*LPGetKeyStatus) (EKEYCOFE kcode);	// ���������� ����� �������� ��������� ������� ��� 0 ���� �� ������


const unsigned char C_IBUFSIZEFRAME = 52;                           // ����������� ������ MTU ������ ���
const unsigned short C_UART_BUF_SIZE = C_IBUFSIZEFRAME * 4;			// ������������ ������ ������ ��� UART ������ TUARTSTAFF ������
const unsigned char C_BSTAFF = 0x08;
const unsigned char C_BST_START = 0x01;
const unsigned char C_BST_STOP = 0x02;
enum E_STAFFCMDS {E_SFCMD_SYNC = 1, E_SFCMD_SYNC_STAFF = 2, E_SFCMD_DATA = 3, E_SFCMD_DATA_STAFF = 4};
enum EDEVSUBCODE {EBASERAM = 0, EEXTFLASH = 1};
enum E_UARTFECMDS {E_FE_SYNC = 1, E_FE_SIZE = 2, E_FE_CRC = 3, E_FE_DATA = 4};

const unsigned long C_UP_PROTOCOL_SPEED = 921600;//115200;
const unsigned long C_TOUCHUART_SPEED = 115200;

const unsigned short C_TimeTxAvaria = 500;
const unsigned char C_TimeOutWrBuf = 50;        // ms - �������� ������������� ������ �� ��������� ����� SLAVE ����������
const unsigned char C_TimeOutRdBuf = 50;        // ms - ������ ������������� ������ �� ���������� ������ SLAVE ����������
const unsigned short C_TimeOutErasse = 500;     // ������������ ����� ����������� �������� ������

#define USRT_BF_FE 2			// ������ ������
#define USRT_BF_NF 4			// ������ - ��� �� �����
#define USRT_BF_RXNE 32
#define USRT_BF_TXE 128


#define C_CMDMASK 0x7F				// ����� �� ��� �������
#define C_CMDMASK_NOERR 0x80		// ����� �� ��� ������ � ���� �������

typedef struct {
	void *lRam;
	unsigned long sizes;
} BF_PAR;


// �������� ������ ������� � ������������

#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	unsigned char CMD;					// ���� ������� �������	(������� ��� �������)
	unsigned char DeviceAdress;		    // ����� ���������� (0 - ��� ��� �����������, 0xFF - �����������������)
	unsigned char DeviceType;			// ��� ���������� ��������� �����/���������/����������/������		
} PRECMDHEADER;



#ifdef  NOBORLAND
typedef __packed struct _STDDEVASK{
#else
typedef struct _STDDEVASK{
#endif
	unsigned char CMD_Result;			//	������� ��� �������� ��� ������� ����������� ������� 
	unsigned char DeviceAdress;		
	unsigned char DeviceType;
} UC_ACK;



// � ��������� �������� ������������ �������� ���� ��������� ��������� ���������
// ��� ����� ���� ������ ���� ������ � �������� ����������

// PRECMDHEADER.CMD = ���� ������� �������

#define C_UC_STATUS 0						// ������ ������� ��������� ����������
#define C_UC_GET_INFO 1					// ���������� � ������� � �������� ��������� �������� � RAM ������
#define C_UC_WRBF 2							// ���������� ��������� �����
#define C_UC_RDBF 3							// ������ ��������� �����
#define C_UC_WRDATA 4 					// ���������� RAM �������� ����������
#define C_UC_RDDATA 5						// ������ RAM �������� ����������
#define C_UC_ERASE 6						// ������� ���-�� �� ���� ����������
#define C_UC_JMPTO_USER 7		
#define C_UC_JMPTO_BOOT 8		
#define C_UC_BRIDGETRANSACTION 9	



// ��������� ������������ � ��������� C_UC_WRBF, C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	unsigned short BufOfs;
	unsigned short sizes;
} BUFINT_ADR;		



// ��������� ������������ � ��������� C_UC_WRDATA, C_UC_RDDATA
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	unsigned long ExtAdress;
	unsigned short ExtSizeData;
} EXTMEM_ADR;

// #################################################################################
// ###############################   C_UC_WRBF   ###################################
// #################################################################################
// �������� - �������� ������ � ��������� �����
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	BUFINT_ADR PARAM;
	// ��� ��������� ������ ������� ���� ��������� � �����
} UPCMD_WRIB;

// ����� �� �������� C_UC_WRBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	BUFINT_ADR PARAM;	// ������� ������ ������� ����������
} UPACK_WRIB;
// #################################################################################


// #################################################################################
// ################################## C_UC_RDBF ####################################
// #################################################################################
// ������� ������ ������ �� ������
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	BUFINT_ADR PARAM;
} UPCMD_RDIB;

// ����� �� �������� C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	BUFINT_ADR PRAMDATA;
	// ��� ����� ����������� �� ������ ������
} UPACK_RDIB;
// #################################################################################


// #################################################################################
// ################################## C_UC_WRDATA ##################################
// #################################################################################
// ������� ������ ������ �� ���������� ������ � ��������� ������� ��� ����������
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
    PRECMDHEADER CD;
    EXTMEM_ADR PARAM;
} UPCMD_WREXT;


// ����� �� �������� C_UC_WRDATA
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
    UC_ACK CD;
		EXTMEM_ADR PARAM;
} UPACK_WREXT;
// #################################################################################



// #################################################################################
// ################################## C_UC_RDDATA ##################################
// #################################################################################
// ������� ������ ������ �� �������� ������� ��� ���������� �� ���������� �����
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	EXTMEM_ADR PARAM;
} UPCMD_RDEXT;



// ����� �� �������� C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	EXTMEM_ADR PARAM;
	// ��� ����� ����������� �� ������ ������
} UPACK_RDEXT;
// #################################################################################


// #################################################################################
// ################################## C_UC_ERASE ###################################
// #################################################################################
// ������� �������� ���������� ��� ��������� ������ ����������
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	unsigned long SectorAdress;
} UPCMD_ERASE;



// ����� �� �������� C_UC_ERASE
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
} UPACK_ERASE;
// #################################################################################


// #################################################################################
// #################  C_UC_JMPTO_USER ############C_UC_JMPTO_BOOT ##################
// #################################################################################
// ������� �������� �� ������������ � ��� ��� � USER programm
// ����� �� ������������
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
} UPCMD_JUMPING;
// #################################################################################



// #################################################################################
// ################################## C_UC_STATUS ##################################
// #################################################################################
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
} UPCMD_STATUS;



#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	unsigned char Plane;	// ���=0 ��� ���������=1
} UPACK_STATUS;
// #################################################################################



// #################################################################################
// ########################### C_UC_BRIDGETRANSACTION ##############################
// #################################################################################
// ������ �� ���������� ������ ���������� �� ���������� � ��������� ����� ���� ������ ������� ������
// ���������� ������� � ��������� ������
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	unsigned short TxSizes;
	unsigned short TimeOut;
} UPCMD_GATE;


#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	unsigned short TxSizes;	// ������ �������� ������, ������� ���� ������� �� ���������� ����� �� ��������� ������
	// � ��������� ������ ���� = 0, �� ��� �������� ���������� ���������� �� ��������
} UPACK_GATE;
// #################################################################################


// ---------------------------------------------------------------- ST8 --------------------------------------------------------------------
/*
		�������� �������� FE, ���������� ���������� ������. ������� ������� �� 3-� ������.
		1. ���������� ���������� � ������� UART FE �� �������� ������� (�������������). �� ������ FE, �������� ����������
		���������/������� ������ ������ � 0. 
		2. ������ ��� ����� � �������� ������ ����� �������������, ��� ��������� "TSTUDATA".
			���� "CountByte" ������ ����������� ������ ��������� �� ���������� "TSTUDATA", �������� �������� ����������� ����� �� �����.
		3. ������ ��������� ��������� ����� ������� �� ���������� ����, ������� ��������� � ���� CountByte, ����� ������������� ������������ �����.
			��� ���������� ����������� �����, �������� ����� ����������� � ��������������� ��������������� ����� � ���������.
			
		���� �������� SLAVE, ������ �������� �� �������� MASTER ����������, �� ��������� �������� ������ � ��� �� ������� ��� � MASTER, ��������� ����
		������ ���������� TSTUDATA. � ����� ���� ��������, ��� SLAVE ���������� �� �������� ������������� FE, ������ ���������� � ������ �������� ������.
*/
// TCHCMDPREFIX
#define KEYCMD_GET 1
#define KEYCMD_SCANTIME 2

// TCHACKPOSTFIX
#define KEYACK_SUCCESS 0

typedef struct {
  unsigned char CountByte;		// ������ ������ ���������� � �����, ������ ���� ��������� �� �����������
  unsigned char CRC8;
} TSTUDATA;



#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
  unsigned short KeyADC_min[3];        
  unsigned short KeyADC_max[3];             
  unsigned char KeyStat[3];             // ���������� �������
  unsigned char DataCnt;                // ������� ���������� �������� ��� ����������
} TTOUCHDATA;


typedef struct {
	unsigned char Code;
} TCHCMDPREFIX;

typedef struct {
	unsigned char Code;
} TCHACKPOSTFIX;



// ---- ������� ������ � TOUCH ������ ----
// ----------------- GET -----------------
typedef struct {
	TCHCMDPREFIX CMD;		// KEYCMD_GET
} TCH_CMD_GET;


typedef struct {
	TCHACKPOSTFIX Result;
	TTOUCHDATA Data;
} TCH_ACK_GET;
// ---------------------------------------
// ------------ SCAN TIME ----------------
typedef struct {
	TCHCMDPREFIX CMD;		// KEYCMD_SCANTIME
	unsigned short Time;
} TCH_CMD_SCANTIME;


typedef struct {
	TCHACKPOSTFIX Result;
} TCH_ACK_SCANTIME;
// ---------------------------------------

#endif

