#ifndef __H_CORE_TYPES_DEFINE_H__
#define __H_CORE_TYPES_DEFINE_H__

typedef enum {EKEY_LEFT = 0, EKEY_MIDL = 1, EKEY_RIGHT = 2, EKEY_MAXCOUNTKEY = 3} EKEYCOFE;
typedef unsigned short (*LPGetKeyStatus) (EKEYCOFE kcode);	// возвращает время нажатого состояния клавиши или 0 если не нажата


const unsigned char C_IBUFSIZEFRAME = 52;                           // размерность записи MTU пакета для
const unsigned short C_UART_BUF_SIZE = C_IBUFSIZEFRAME * 4;			// определяется размер буфера для UART данных TUARTSTAFF класса
const unsigned char C_BSTAFF = 0x08;
const unsigned char C_BST_START = 0x01;
const unsigned char C_BST_STOP = 0x02;
enum E_STAFFCMDS {E_SFCMD_SYNC = 1, E_SFCMD_SYNC_STAFF = 2, E_SFCMD_DATA = 3, E_SFCMD_DATA_STAFF = 4};
enum EDEVSUBCODE {EBASERAM = 0, EEXTFLASH = 1};
enum E_UARTFECMDS {E_FE_SYNC = 1, E_FE_SIZE = 2, E_FE_CRC = 3, E_FE_DATA = 4};

const unsigned long C_UP_PROTOCOL_SPEED = 921600;//115200;
const unsigned long C_TOUCHUART_SPEED = 115200;

const unsigned short C_TimeTxAvaria = 500;
const unsigned char C_TimeOutWrBuf = 50;        // ms - загрузка максимального пакета во внутрений буфер SLAVE устройства
const unsigned char C_TimeOutRdBuf = 50;        // ms - чтение максимального пакета из внутренего буфера SLAVE устройства
const unsigned short C_TimeOutErasse = 500;     // максимальное время физического стирания памяти

#define USRT_BF_FE 2			// ошибка фрейма
#define USRT_BF_NF 4			// ошибка - шум на линии
#define USRT_BF_RXNE 32
#define USRT_BF_TXE 128


#define C_CMDMASK 0x7F				// маска на код команды
#define C_CMDMASK_NOERR 0x80		// маска на бит ошибки в коде команды

typedef struct {
	void *lRam;
	unsigned long sizes;
} BF_PAR;


// протокол обмена данными с устройтсвами

#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	unsigned char CMD;					// коды базовых комманд	(старший бит сброшен)
	unsigned char DeviceAdress;		    // адрес устройства (0 - это усб координатор, 0xFF - широковещательная)
	unsigned char DeviceType;			// тип внутренего условного блока/периферии/интерфейса/памяти		
} PRECMDHEADER;



#ifdef  NOBORLAND
typedef __packed struct _STDDEVASK{
#else
typedef struct _STDDEVASK{
#endif
	unsigned char CMD_Result;			//	старший бит означает что команда выполнилась успешно 
	unsigned char DeviceAdress;		
	unsigned char DeviceType;
} UC_ACK;



// в некоторых командах присутствуют условные коды адресации внутрених устройств
// это может быть разные типы памяти и выходные интерфейсы

// PRECMDHEADER.CMD = коды базовых комманд

#define C_UC_STATUS 0						// читает простую статусную информацию
#define C_UC_GET_INFO 1					// информация о входной и выходной командных структур в RAM памяти
#define C_UC_WRBF 2							// записывает внутрений буфер
#define C_UC_RDBF 3							// читает внутрений буфер
#define C_UC_WRDATA 4 					// записывает RAM базового устройства
#define C_UC_RDDATA 5						// читает RAM базового устройства
#define C_UC_ERASE 6						// стирает что-то по коду устройства
#define C_UC_JMPTO_USER 7		
#define C_UC_JMPTO_BOOT 8		
#define C_UC_BRIDGETRANSACTION 9	



// структура используемая в коммандах C_UC_WRBF, C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	unsigned short BufOfs;
	unsigned short sizes;
} BUFINT_ADR;		



// структура используемая в коммандах C_UC_WRDATA, C_UC_RDDATA
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
// Комманда - записать данные в внутрений буфер
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	BUFINT_ADR PARAM;
	// тут находятся данные которые надо поместить в буфер
} UPCMD_WRIB;

// ответ на комманду C_UC_WRBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	BUFINT_ADR PARAM;	// сколько данные реально записались
} UPACK_WRIB;
// #################################################################################


// #################################################################################
// ################################## C_UC_RDBF ####################################
// #################################################################################
// Команда чтения данных из буфера
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	BUFINT_ADR PARAM;
} UPCMD_RDIB;

// ответ на комманду C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	BUFINT_ADR PRAMDATA;
	// тут будут прочитанные из буфера данные
} UPACK_RDIB;
// #################################################################################


// #################################################################################
// ################################## C_UC_WRDATA ##################################
// #################################################################################
// Команда записи данных из внутренего буфера в указанную область или устройство
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
    PRECMDHEADER CD;
    EXTMEM_ADR PARAM;
} UPCMD_WREXT;


// ответ на комманду C_UC_WRDATA
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
// Команда чтения данных из указаной области или устройства во внутренний буфер
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	EXTMEM_ADR PARAM;
} UPCMD_RDEXT;



// ответ на комманду C_UC_RDBF
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	UC_ACK CD;
	EXTMEM_ADR PARAM;
	// тут будут прочитанные из буфера данные
} UPACK_RDEXT;
// #################################################################################


// #################################################################################
// ################################## C_UC_ERASE ###################################
// #################################################################################
// Стирает выбраное устройство или выбранный сектор устройства
#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
	PRECMDHEADER CD;
	unsigned long SectorAdress;
} UPCMD_ERASE;



// ответ на комманду C_UC_ERASE
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
// команды перехода на подпрограммы в бут или в USER programm
// ответ не предусмотрен
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
	unsigned char Plane;	// бут=0 или программа=1
} UPACK_STATUS;
// #################################################################################



// #################################################################################
// ########################### C_UC_BRIDGETRANSACTION ##############################
// #################################################################################
// данные из внутренего буфера передаются на устройство и ожидается ответ если указан таймаут приема
// устройство указано в заголовке пакета
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
	unsigned short TxSizes;	// размер принятых данных, которые были ответом на транзакцию лежат во внутренем буфере
	// в противном случае если = 0, то это означает завершение транзакции по таймауту
} UPACK_GATE;
// #################################################################################


// ---------------------------------------------------------------- ST8 --------------------------------------------------------------------
/*
		Протокол передачи FE, реализация канального уровня. Образно состоит из 3-х этапов.
		1. Транзакция начинается с посылки UART FE на приемную сторону (синхронизации). По приему FE, приемник сбрасывает
		указатель/счетчик приема пакета в 0. 
		2. Первые два байта в байтовом потоке после синхронизации, это структура "TSTUDATA".
			Поле "CountByte" хранит размерность данных следующих за структурой "TSTUDATA", алгоритм подсчета контрольной суммы на выбор.
		3. Данные считаются принятыми когда принято то количество байт, которое заявленно в поле CountByte, после высчитывается контрольнная сумма.
			При совпадении контрольной суммы, приемный буфер блокируется и устанавливаются соответствующие флаги и состояния.
			
		Если приемник SLAVE, должен ответить на комманду MASTER устройству, он формирует ответные данные в том же порядке что и MASTER, предваряя блок
		данных структурой TSTUDATA. С одним лишь отличием, что SLAVE устройство не посылает синхронизацию FE, мастер устройству в начале отправки ответа.
*/
// TCHCMDPREFIX
#define KEYCMD_GET 1
#define KEYCMD_SCANTIME 2

// TCHACKPOSTFIX
#define KEYACK_SUCCESS 0

typedef struct {
  unsigned char CountByte;		// размер данных включаемых в пакет, размер этой структуры не учитывается
  unsigned char CRC8;
} TSTUDATA;



#ifdef  NOBORLAND
typedef __packed struct {
#else
typedef struct {
#endif
  unsigned short KeyADC_min[3];        
  unsigned short KeyADC_max[3];             
  unsigned char KeyStat[3];             // дискретные статусы
  unsigned char DataCnt;                // счетчик полученных отсчетов для статистики
} TTOUCHDATA;


typedef struct {
	unsigned char Code;
} TCHCMDPREFIX;

typedef struct {
	unsigned char Code;
} TCHACKPOSTFIX;



// ---- команды обмена с TOUCH платой ----
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

