#ifndef _H_CAN_CORE_LIB_H_
#define _H_CAN_CORE_LIB_H_

#include "stm32f10x.h"
#include "stm32f10x.h"
#include "stm32f10x_can.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "rutine.h"
#include "uavcan.h"

#include "canard_stm32.h"
#include "sysbios.h"
#include "canard.h"
#include "Temperature.h"
#include "GetNodeInfo.h"
#include "RestartNode.h"
#include "BeginFirmwareUpdate.h"
#include "Measurement.h"
#include "Delete.h"
#include "GetInfo.h"
#include "Read.h"
#include "Write.h"
#include "EntryType.h"
#include "Path.h"
#include "main.h"
#include "tuavparams.h"
#include "RawCommand.h"
#include "BatteryInfo.h"
#include "CircuitStatus.h"
#include "arrayCommand.h"
#include "ArmingStatus.h"
#include "TTFIFO.h"


// call back заглушка на подтверждение - что пакет передан (пока не используется)
extern bool shouldAcceptTransfer(const CanardInstance* ins, uint64_t* out_data_type_signature, uint16_t data_type_id, CanardTransferType transfer_type, uint8_t source_node_id);
//extern TWINBOND25X16 ExtFlash;


#ifdef __cplusplus
 extern "C" {
#endif 

	 
#include "canard.h"
	 

 
#ifdef __cplusplus
}
#endif


extern void uavcanInit(void);

#define SOURCE_ID_FROM_ID(x)                        ((uint8_t) (((x) >> 0U)  & 0x7FU))
#define SERVICE_NOT_MSG_FROM_ID(x)                  ((bool)    (((x) >> 7U)  & 0x1U))
#define REQUEST_NOT_RESPONSE_FROM_ID(x)             ((bool)    (((x) >> 15U) & 0x1U))
#define DEST_ID_FROM_ID(x)                          ((uint8_t) (((x) >> 8U)  & 0x7FU))
#define PRIORITY_FROM_ID(x)                         ((uint8_t) (((x) >> 24U) & 0x1FU))
#define MSG_TYPE_FROM_ID(x)                         ((uint16_t)(((x) >> 8U)  & 0xFFFFU))
#define SRV_TYPE_FROM_ID(x)                         ((uint8_t) (((x) >> 16U) & 0xFFU))



// ardupilot.measure.POWERBRD
#define UAVCAN_POWERBOARD_MEASUREMENT_ID 20795
#define UAVCAN_POWERBOARD_MEASUREMENT_SIGNATURE 0x25B131234B861657ULL


#define C_UAVCAN_RDFTECHWAIT_62MS 70
#define C_UAVCAN_READFILE_TIMEOUT_MS 120
#define C_UAVCAN_READFILE_REPEAT 50

//const unsigned char C_CURENT_DEVID = 49;


class TCANIFACE {
	public:
		virtual void ISR_rx () = 0;
		virtual void ISR_tx () = 0;			
};



typedef struct {
	float range;
	float min_dist;
	float max_dist;
	bool error;
	bool f_is_data;
} SUAV_LIDAR;



typedef struct {
	float celsium;
	bool error;
	bool f_is_data;
}SUAV_TEMPERATURE;



typedef struct {
	float pascal;
	bool error;
	bool f_is_data;
}SUAV_PRESSURE;







typedef struct {
	uavcan_protocol_GetNodeInfoResponse frame;
	bool error;
	bool f_is_data;
} SUAV_GETNODEINFO_DATA;

#define C_RTCMUAVCANBUFSIZE 255



typedef enum UAVCAN_NODE_HEALTH
{
   UAVCAN_NODE_HEALTH_OK=0, /* The node is functioning properly. | */
   UAVCAN_NODE_HEALTH_WARNING=1, /* A critical parameter went out of range or the node has encountered a minor failure. | */
   UAVCAN_NODE_HEALTH_ERROR=2, /* The node has encountered a major failure. | */
   UAVCAN_NODE_HEALTH_CRITICAL=3, /* The node has suffered a fatal malfunction. | */
   UAVCAN_NODE_HEALTH_ENUM_END=4, /*  | */
} UAVCAN_NODE_HEALTH;

typedef enum UAVCAN_NODE_MODE
{
   UAVCAN_NODE_MODE_OPERATIONAL=0, /* The node is performing its primary functions. | */
   UAVCAN_NODE_MODE_INITIALIZATION=1, /* The node is initializing; this mode is entered immediately after startup. | */
   UAVCAN_NODE_MODE_MAINTENANCE=2, /* The node is under maintenance. | */
   UAVCAN_NODE_MODE_SOFTWARE_UPDATE=3, /* The node is in the process of updating its software. | */
   UAVCAN_NODE_MODE_OFFLINE=7, /* The node is no longer available online. | */
   UAVCAN_NODE_MODE_ENUM_END=8, /*  | */
} UAVCAN_NODE_MODE;

enum E_HEALTH {E_HEALTH_OK = UAVCAN_NODE_HEALTH_OK, E_HEALTH_WARNING = UAVCAN_NODE_HEALTH_WARNING, E_HEALTH_ERROR = UAVCAN_NODE_HEALTH_ERROR, E_HEALTH_CRITICAL = UAVCAN_NODE_HEALTH_CRITICAL};
enum E_NDMODE {E_NDMODE_OPERATIONAL = UAVCAN_NODE_MODE_OPERATIONAL, E_NDMODE_INITIALIZATION = UAVCAN_NODE_MODE_INITIALIZATION, E_NDMODE_SOFTWARE_UPDATE = UAVCAN_NODE_MODE_SOFTWARE_UPDATE};
enum E_CANMSGTYP {E_CANMSGTYP_DEBUG = 0, E_CANMSGTYP_INFO = 1, E_CANMSGTYP_WARNING = 2, E_CANMSGTYP_ERROR = 3, E_CANMSGTYP_ENDENUM = 4};

#define C_MESSAGE_TAGS_MAX 2

class TMESGTAG {
		char message[UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE];
		E_CANMSGTYP type;
		unsigned char len;
		bool f_message;
		
	public:
		TMESGTAG ();
		bool add_message (char *ltxt_in, E_CANMSGTYP type);
		bool check_message ();
		unsigned char get_message (char **ltxt_in, E_CANMSGTYP &d_type);
		unsigned char lenght ();
};


class TMESSAGECAN {
		TMESGTAG tag[C_MESSAGE_TAGS_MAX];
		unsigned char push_ix;
		unsigned char pop_ix;
		unsigned char cnt;

	public:
		TMESSAGECAN ();
		bool add_message (char *ltxt_in, E_CANMSGTYP d_type);
		unsigned char get_message (char **ltxt_in, E_CANMSGTYP &d_type);
		bool check_messages ();
};



class TCANISR {
	protected:
		static TTFIFO<CanardCANFrame> *fifo;
	
	public:
		static void isr_rx ();
};


class TUAVCAN : public TCANIFACE, public TCANISR {
	private:

		uint8_t g_canard_memory_pool[2048];
		TMESSAGECAN mesagecan;
		utimer_t PingCanard_timer;
		utimer_t FirmReadReq_timer;
		void Send_raw (char *lSrc, unsigned char sizes);
	
		uint8_t Cur_NODE_ID;

		void PushQUE_Status ();

		virtual void ISR_rx ();
		virtual void ISR_tx ();

		void RecvFrameDispatchTiles(void);
		static void hardCB_recvFrame (void *lpthis, CanardInstance* ins, CanardRxTransfer* transfer);
		void CB_FullRecvFrame (CanardInstance* ins, CanardRxTransfer* transfer);
		bool ParseUAVCANCommandFromMaster ();

		long ReadFileReq (char *FlName, unsigned char FlName_size, unsigned long offs_bytes);

	protected:

		//uint8_t transfer_id;
		uint8_t read_file_transfer_id;

		SUAV_GETNODEINFO_DATA UavGetNodeInfo;
		uint32_t GetNodeInfo_UAVCANFrame (uavcan_protocol_GetNodeInfoResponse *source, void* msg_buf, uint32_t offset);
		long UAVCAN_ParseGetInfo_resp (CanardRxTransfer *transfer);
		long UAVCAN_GetInfo_req (char *lFileName, unsigned char sz_name);

		unsigned char FirmBufData_RD[UAVCAN_PROTOCOL_FILE_READ_RESPONSE_MAX_SIZE];

		unsigned char FirmWareName[64];
		unsigned char FirmNameSize;
		bool f_NoInfo_FileSize;			// mision planer не поддерживает команду uavcan GetFileInfo  (она возвращает размер файла)
		long FirmFileSize;
		unsigned long FirmLoadByteOffs;
		unsigned char FirmSw;
		unsigned char FirmRepCount;
		uint8_t FirmNodeSrc;

		void Firmware_SubTask ();
		//unsigned char read_file_id;

		static E_HEALTH curHealtStatus;
		static E_NDMODE curNodeMode;

		unsigned long dbg_lastprocent;
		void DMessage_SubTask ();
		unsigned long c_uavlog_period;

		virtual void user_task () = 0;
		virtual void CB_UserRecvFrame (CanardInstance* ins, CanardRxTransfer* transfer) = 0;

	public:
		TUAVCAN ();
	
		char DebugMessBuf[UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH];
	
		void Init (unsigned char nodID);
		void Task ();
		bool GetRx_recv_status ();		// есть ли принятые данные
		bool GetTX_free_status ();		// свободен ли передатчик
	
		void SetUavLogTime (unsigned long tm);
		void MessageSend_info (const char *lStr, unsigned long sz);
		void MessageSend_warning (const char *lStr, unsigned long sz);
		void MessageSend_error (const char *lStr, unsigned long sz);
		bool is_message_free ();
		
		
		static void SetHealtStatus (E_HEALTH stat);
		static E_HEALTH GetHealtStatus ();
		
		static void SetNodeMode (E_NDMODE md);
		static void SetNodeID (unsigned char nod_idd);
		static E_NDMODE GetNodeMode ();
		
		static uint8_t GetVersion_MAJOR ();
		static uint8_t GetVersion_Minor ();
		static uint32_t GetProgram_CRC ();
		
		bool CheckFirmwareStatus ();
};




#endif


