#include "cancore.h"
#include "stm32f10x_can.h"
#include "rutine.h"
#include "BeginFirmwareUpdate.h"
#include "Read.h"
#include "GetInfo.h"
#include "sectors.h"
#include "ExecuteOpcode.h"
#include "main.h"
#include "LogLevel.h"
#include "STMSTRING.h"



#ifndef CANARD_INTERNAL_SATURATE_UNSIGNED
#define CANARD_INTERNAL_SATURATE_UNSIGNED(x, max) ( ((x) >= max) ? max : (x) );
#endif

#ifndef CANARD_INTERNAL_SATURATE
#define CANARD_INTERNAL_SATURATE(x, max) ( ((x) > max) ? max : ( (-(x) > max) ? (-max) : (x) ) );
#endif

extern void uavcanInit (void);
extern unsigned long millis (void);


const char *ltpmsg_inf = "D";		
const char *ltpmsg_warn = "Warn";
const char *ltpmsg_err = "Error";



E_HEALTH TUAVCAN::curHealtStatus = E_HEALTH_OK;
E_NDMODE TUAVCAN::curNodeMode = E_NDMODE_OPERATIONAL;


// ---------------------------------------
TMESGTAG::TMESGTAG ()
{
	len = 0;
	f_message = false;
}



bool TMESGTAG::add_message (char *ltxt_in, E_CANMSGTYP tp)
{
	bool rv = false;
	if (ltxt_in && !f_message)
		{
		unsigned long sz = lenstr (ltxt_in);
		if (sz)
			{
			if (sz >= sizeof(message)) sz = sizeof(message) - 1;
			CopyMemorySDC (ltxt_in, message, sz);
			type = tp;
			len = sz;
			message[sz] = 0;
			rv = true;
			f_message = true;
			}
		}
	return rv;
}



bool TMESGTAG::check_message ()
{
	return f_message;
}



unsigned char TMESGTAG::get_message (char **ltxt_in, E_CANMSGTYP &d_type)
{
	unsigned char rv = 0;
	if (f_message)
		{
		if (ltxt_in) *ltxt_in = message;//CopyMemorySDC (message, ltxt_in, len);
		d_type = type;
		rv = len;
		f_message = false;
		}
	return rv;
}



unsigned char TMESGTAG::lenght ()
{
	unsigned char rv = 0;
	if (f_message) rv = len;
	return rv;
}



// --------------------------------
TMESSAGECAN::TMESSAGECAN ()
{
push_ix = 0;
pop_ix = 0;
cnt = 0;
}



bool TMESSAGECAN::add_message (char *ltxt_in, E_CANMSGTYP d_type)
{
	bool rv = false;
	if (ltxt_in && cnt < C_MESSAGE_TAGS_MAX)
		{
		if (push_ix >= C_MESSAGE_TAGS_MAX) push_ix = 0;
		rv = tag[push_ix].add_message (ltxt_in, d_type);
		push_ix++;
		cnt++;
		}
	return rv;
}



unsigned char TMESSAGECAN::get_message (char **ltxt_dst, E_CANMSGTYP &d_type)
{
	unsigned char rv = 0;
	if (cnt)
		{
		if (pop_ix >= C_MESSAGE_TAGS_MAX) pop_ix = 0;
		rv = tag[pop_ix].get_message (ltxt_dst, d_type);
		pop_ix++;
		cnt--;
		}
	return rv;
}



bool TMESSAGECAN::check_messages ()
{
	return cnt;
}
	



#ifdef __cplusplus
 extern "C" {
#endif 


void USB_LP_CAN1_RX0_IRQHandler ()
{
	if (CAN_GetITStatus (CAN1, CAN_IT_FMP0) == SET)
		{
		TCANISR::isr_rx ();	
		CAN_ClearITPendingBit (CAN1, CAN_IT_FMP0);
		}	
	if (CAN_GetFlagStatus (CAN1, CAN_IT_FMP1) == SET) 
		{
		TCANISR::isr_rx ();
		CAN_ClearITPendingBit (CAN1, CAN_IT_FMP1);
		}
}


#ifdef __cplusplus
}
#endif

TTFIFO<CanardCANFrame> *TCANISR::fifo = 0;

void TCANISR::isr_rx ()
{
	CanardCANFrame rx_frame;
  int res = canardSTM32Receive (&rx_frame);
	if (res) {
		if (fifo) fifo->push (&rx_frame);
		}
}



// --------------------------------
TUAVCAN::TUAVCAN ()
{
	fifo = new TTFIFO <CanardCANFrame>(64);	
	PingCanard_timer = 0;
	FirmReadReq_timer = 0;
	SYSBIOS::ADD_TIMER_SYS (&PingCanard_timer); 
	SYSBIOS::ADD_TIMER_SYS (&FirmReadReq_timer); 
	c_uavlog_period = 0;
}



void TUAVCAN::SetUavLogTime (unsigned long tm)
{
	c_uavlog_period = tm;
}



void TUAVCAN::SetHealtStatus (E_HEALTH stat)
{
	curHealtStatus = stat;
}



E_HEALTH TUAVCAN::GetHealtStatus ()
{
	return curHealtStatus;
}


	
void TUAVCAN::Init (unsigned char nodID)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;
	Cur_NODE_ID = nodID;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_PinRemapConfig (GPIO_Remap1_CAN1, ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	
	
  CanardSTM32CANTimings timings;
	
  int result = canardSTM32ComputeCANTimings(HAL_RCC_GetPCLK1Freq(), 1000000, &timings);
  if (result) {
    __ASM volatile("BKPT #01");
  }
  result = canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
  if (result) {
    __ASM volatile("BKPT #01");
  }

  canardInit(&g_canard,                         // Uninitialized library instance
             g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation - указатель на выделенный буфер 2 кбайта
             sizeof(g_canard_memory_pool),      // Size of the above, in bytes									- размер буфера
             (CanardOnTransferReception)hardCB_recvFrame,                // Callback, see CanardOnTransferReception		
             shouldAcceptTransfer,              // Callback, see CanardShouldAcceptTransfer
             this);
  canardSetLocalNodeID(&g_canard, nodID);			// 44   13.12.19  C_CURENT_DEVID
						 
	CAN_ITConfig (CAN1, CAN_IT_FMP0, ENABLE);
	CAN_ITConfig (CAN1, CAN_IT_FMP1, ENABLE);					 
						 
	NVIC_InitTypeDef NVIC_InitStructure;	
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
						 
  //NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //NVIC_Init(&NVIC_InitStructure);
}



void TUAVCAN::SetNodeID (unsigned char nod_idd)
{
	canardSetLocalNodeID (&g_canard, nod_idd);	
}



bool TUAVCAN::GetTX_free_status ()
{
	return !canardSTM32_TxStatus ();
}



bool TUAVCAN::GetRx_recv_status ()
{
	return canardSTM32_RxStatus ();
}




void TUAVCAN::Task ()
{
	user_task ();
	
	if (GetTX_free_status ()) 
		{
		if (TUAVCAN::GetNodeMode () != E_NDMODE_SOFTWARE_UPDATE)	// если идет обновление прошивки, то отключить отправку сообщений
			{
			/*
			if (lp_new_batinf_mess)		// готовность данных и период формирования задается в файле tbatcontrol.cpp
				{
				PushQUE_Battinf ();
				lp_new_batinf_mess = 0;
				}

			if (lp_new_circuit_status_mess)			// готовность данных и период формирования задается в файле tbatcontrol.cpp
				{
				PushQUE_CircuitStatus ();
				lp_new_circuit_status_mess = 0;
				}
			*/
			DMessage_SubTask ();
			}
		else
			{
			// отображение процесса обновления прошивки в виде debug сообщений.
			DMessage_SubTask ();
			}
		if (!PingCanard_timer)
			{
			PushQUE_Status ();
			PingCanard_timer = 1000;
			}
		}
	Firmware_SubTask ();
	RecvFrameDispatchTiles();
	SendCanard();
}



// контролирует сборку полного кадра и запускает CB
void TUAVCAN::RecvFrameDispatchTiles(void)
{
  CanardCANFrame rx_frame;
  //int res = canardSTM32Receive(&rx_frame);
	if (fifo)
		{
		while (fifo->frame_count ())
			{	
			fifo->pop (rx_frame);
			canardHandleRxFrame(&g_canard, &rx_frame, HAL_GetTick() * 1000);
			}
		/*
		if (fifo->frame_count ())
			{
			fifo->pop (rx_frame);
			while (res)
				{
				canardHandleRxFrame(&g_canard, &rx_frame, HAL_GetTick() * 1000);
				res = canardSTM32Receive(&rx_frame);
				}
			}	
			*/
		}
	/*
  if (res) {
    canardHandleRxFrame(&g_canard, &rx_frame, HAL_GetTick() * 1000);
		}
	*/
}



/*
void TUAVCAN::PushQUE_Battinf ()
{
  uint8_t buffer[UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE];
  uint32_t offset = 0;
	

	offset = uavcan_equipment_power_BatteryInfo_encode_internal (lp_new_batinf_mess, buffer, offset, 0);

  canardBroadcast(&g_canard,
                  UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE, 
                  UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOWEST, 
                  &buffer[0],
                  (offset + 7) / 8);
}



void TUAVCAN::PushQUE_CircuitStatus ()
{
  uint8_t buffer[UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_MAX_SIZE];
  uint32_t offset = 0;
	

	offset = uavcan_equipment_power_CircuitStatus_encode_internal (lp_new_circuit_status_mess, buffer, offset, 1);

  canardBroadcast(&g_canard,
                  UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_SIGNATURE, 
                  UAVCAN_EQUIPMENT_POWER_CIRCUITSTATUS_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOWEST, 
                  &buffer[0],
                  (offset + 7) / 8);
}

*/



void TUAVCAN::PushQUE_Status ()
{
  uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];
  static uint8_t transfer_id = 0;
  makeNodeStatusMessage(buffer);
  canardBroadcast(&g_canard,
                  UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
                  UAVCAN_NODE_STATUS_DATA_TYPE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW , 
                  buffer,
                  UAVCAN_NODE_STATUS_MESSAGE_SIZE);             //some indication
}








/*
void TUAVCAN::SetTemperatureData (float dat, bool error)
{
	UavTemperature1.celsium = dat;
	UavTemperature1.error = error;
	UavTemperature1.f_is_data = true;
}
*/




void TUAVCAN::ISR_rx ()
{
}



void TUAVCAN::ISR_tx ()
{
}




void TUAVCAN::hardCB_recvFrame (void *lpthis, CanardInstance* ins, CanardRxTransfer* transfer)
{
	((TUAVCAN*)lpthis)->CB_FullRecvFrame(ins, transfer);
}



//volatile static bool f_test = false;
//static unsigned long rx_firmware_amont = 0;


volatile static unsigned char firmware_namePath = 0;


static unsigned long fr_cnt = 0;
//static unsigned char err = 0;






void TUAVCAN::SetNodeMode (E_NDMODE md)
{
	curNodeMode = md;
}



E_NDMODE TUAVCAN::GetNodeMode ()
{
	return curNodeMode;
}



long TUAVCAN::UAVCAN_ParseGetInfo_resp (CanardRxTransfer *transfer)
{
	long rv = -1;
	uavcan_protocol_file_GetInfoResponse resp_frame;
	unsigned char *lDarr = FirmBufData_RD;
	int32_t len = uavcan_protocol_file_GetInfoResponse_decode (transfer, sizeof(FirmBufData_RD), &resp_frame, &lDarr);
	if (len > 0)
		{
		if (resp_frame.error.value == UAVCAN_PROTOCOL_FILE_ERROR_OK)
			{
			if ((resp_frame.entry_type.flags & UAVCAN_PROTOCOL_FILE_ENTRYTYPE_FLAG_FILE))
				{
				rv = (resp_frame.size);
				}
			}
		}
	return rv;
}



long TUAVCAN::UAVCAN_GetInfo_req (char *lFileName, unsigned char sz_name)
{
uavcan_protocol_file_GetInfoRequest req_frame;
req_frame.path.path.data = FirmWareName;
req_frame.path.path.len = FirmNameSize;
uint8_t buf_tx[UAVCAN_PROTOCOL_FILE_GETINFO_REQUEST_MAX_SIZE];
uint32_t sz = uavcan_protocol_file_GetInfoRequest_encode (&req_frame, buf_tx);
	static uint8_t transfer_id = 0;
  long result = canardRequestOrRespond (&g_canard, FirmNodeSrc, UAVCAN_PROTOCOL_FILE_GETINFO_SIGNATURE, UAVCAN_PROTOCOL_FILE_GETINFO_ID, &transfer_id,
                                      CANARD_TRANSFER_PRIORITY_LOW , CanardRequest, &buf_tx[0], (uint16_t)sz);		//  CANARD_TRANSFER_PRIORITY_LOW
	return result;
}



bool TUAVCAN::CheckFirmwareStatus ()
{
	bool rv = false;
	if (curNodeMode == E_NDMODE_SOFTWARE_UPDATE) rv = true;
	return rv;
}



void TUAVCAN::Firmware_SubTask ()
{
	//if (FirmReadReq_timer) return;
/*
	Механизм получения файла, разбит на несколько частей:
		A). Чтение информации о файле firmware.
			1. Установка количество повторов по ошибкам.
			2. Асинхронный запрос информации о файле. Установка таймаута приема.
			3. Проверка приема и переход на повтор в случае ошибки. Выход с цикла по лимиту ошибок.
	
		Б). Чтение файла:
			4. Установка параметра смещения чтения файла. Количества повторов. И таймаута операции.
	
*/
	
	
	switch (FirmSw)
		{
		case 0:
			{
			break;
			}
		case 1:
			{
			FirmRepCount = 5;			// количесвто повторов для команды GetInfo - размер файла
			FirmReadReq_timer = 0;
			FirmSw++;
			// без break;
			}
		case 2:
			{
			if (FirmReadReq_timer) break;
			long rslt = UAVCAN_GetInfo_req ((char*)FirmWareName, FirmNameSize);
			if (rslt > 0) 
				{
				FirmSw++;							// переходим в режим ожидания приема и проверки таймаута
				FirmReadReq_timer = C_UAVCAN_READFILE_TIMEOUT_MS;		// ждем ответа столько времени
				}
			else
				{
				if (FirmRepCount) 
					{
					FirmReadReq_timer = 10;		// аварийный повтор через 10 мс
					FirmRepCount--;
					}
				else
					{
					f_NoInfo_FileSize = true;
					FirmFileSize = 0xE000;	
					FirmSw = 9;
						
					//TUAVCAN::SetNodeMode ( E_NDMODE_OPERATIONAL);
					//FirmSw = 0;		
					}
				}
			break;
			}
		case 3:
			{
			if (!FirmReadReq_timer)
				{
				// таймаут вышел
				if (FirmRepCount) FirmRepCount--;		// количество повторов
				if (FirmRepCount)
					{
					FirmSw = 2;								// если лимит не вышел - запрашиваем информацию снова
					FirmReadReq_timer = 100;
					}
				else
					{
					f_NoInfo_FileSize = true;
					FirmFileSize = 0xE000;	
					FirmSw = 9;
					//TUAVCAN::SetNodeMode (E_NDMODE_OPERATIONAL);
					//FirmSw = 0;		
					}
				}
			else
				{
				// если за период таймаута данные пришли
				// то внешний код в функции обработки приема 
				// данных с uavcan переключает состояние FirmSw
				}
			break;
			}
		case 5:
			{
			// error 
			break;
			}
		case 9:
			{
			// размер файла прошивки теперь известен,
			// очищаем память для приема файла
			dbg_lastprocent = 0;
			EraseExtF_Place (C_ADREXT_FIRMWARE_T, FirmFileSize);
			//EraseExtF_Place (C_ADREXT_FIRMWARE_T, FirmFileSize);
			FirmSw++;
			// без break.
			}
		case 10:
			{
			FirmRepCount = C_UAVCAN_READFILE_REPEAT;			// количество повторов
			FirmLoadByteOffs = 0;
			FirmReadReq_timer = C_UAVCAN_RDFTECHWAIT_62MS;
			FirmSw++;
			fr_cnt = 0;
			// без break;
			}
		case 11:
			{
			if (FirmReadReq_timer) break;
			//read_file_transfer_id = transfer_id;
			long rslt = ReadFileReq ((char*)FirmWareName, FirmNameSize, FirmLoadByteOffs);
			if (rslt > 0)
				{
				FirmSw++;		// запрос успешно ушел в сеть, переходим на ожидание приема response
				FirmReadReq_timer = C_UAVCAN_READFILE_TIMEOUT_MS;		// ждем ответа столько времени
				}
			else
				{
				if (FirmRepCount) 
					{
					FirmRepCount--;
					FirmReadReq_timer = 0;		// повтор через 10 мс
					}
				else
					{
					FirmSw = 0;							// лимит повторов переходим в неактив
					TUAVCAN::SetNodeMode ( E_NDMODE_OPERATIONAL);
					}
				}
			break;
			}
		case 12:
			{
			if (!FirmReadReq_timer)
				{
				// таймаут вышел
				if (FirmRepCount) FirmRepCount--;		// количество повторов
				if (FirmRepCount)
					{
					FirmSw = 11;			// если лимит не вышел - запрашиваем информацию снова
					}
				else
					{
					FirmSw = 0;			// лимит вышел - переходим в не актив
					TUAVCAN::SetNodeMode ( E_NDMODE_OPERATIONAL);
					}
				}
			else
				{
				// если за период таймаута данные пришли
				// то внешний код в функции обработки приема 
				// данных с uavcan переключает состояние FirmSw
				}
			break;
			}

		case 50:
			{
			// error 
			break;
			}
		}
}



long TUAVCAN::ReadFileReq (char *FlName, unsigned char FlName_size, unsigned long offs_bytes)
{
	uavcan_protocol_file_ReadRequest req_data;
	req_data.offset = offs_bytes;
	req_data.path.path.data = (uint8_t*)FlName;
	req_data.path.path.len = FlName_size;
	
	uint8_t buf_req_enc[UAVCAN_PROTOCOL_FILE_READ_REQUEST_MAX_SIZE];
	uint32_t len = uavcan_protocol_file_ReadRequest_encode (&req_data, buf_req_enc);
	
	static uint8_t read_file_transfer_id = 0;
	
  long result = canardRequestOrRespond (&g_canard, FirmNodeSrc, UAVCAN_PROTOCOL_FILE_READ_SIGNATURE, UAVCAN_PROTOCOL_FILE_READ_ID, &read_file_transfer_id,
                                      CANARD_TRANSFER_PRIORITY_LOWEST , CanardRequest, &buf_req_enc[0], (uint16_t)len);		//  CANARD_TRANSFER_PRIORITY_LOW
	
	return result;
}



void TUAVCAN::MessageSend_info (const char *lStr, unsigned long sz)
{
		if (sz > (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1)) sz = (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1);
		mesagecan.add_message ((char*)lStr, E_CANMSGTYP_INFO);
}


void TUAVCAN::MessageSend_warning (const char *lStr, unsigned long sz)
{
		if (sz > (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1)) sz = (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1);
		mesagecan.add_message ((char*)lStr, E_CANMSGTYP_WARNING);
}


void TUAVCAN::MessageSend_error (const char *lStr, unsigned long sz)
{
		if (sz > (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1)) sz = (UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_TEXT_MAX_LENGTH - 1);
		mesagecan.add_message ((char*)lStr, E_CANMSGTYP_ERROR);
}


/*
void TUAVCAN::message_send_info (const char *lStr)
{
	if (lStr) MessageSend_info (lStr, strlen (lStr));
}
*/


bool TUAVCAN::is_message_free ()
{
	return !mesagecan.check_messages ();
}




void TUAVCAN::DMessage_SubTask ()
{

if (mesagecan.check_messages())	
	{
	unsigned char buffer[UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE];
	uavcan_protocol_debug_LogMessage messg;
	char *lmess;	
	E_CANMSGTYP cur_type;
	unsigned char siztxt = mesagecan.get_message (&lmess, cur_type);
	if (siztxt)
		{
		messg.level.value = cur_type; // UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_INFO;
		messg.text.data = (uint8_t*)lmess;
		messg.text.len = siztxt;
		
		uint8_t *lchn_name = (uint8_t*)ltpmsg_inf;
		switch (cur_type)
			{
			case E_CANMSGTYP_WARNING:
				{
				lchn_name = (uint8_t*)ltpmsg_warn;
				break;
				}
			case E_CANMSGTYP_ERROR:
				{
				lchn_name = (uint8_t*)ltpmsg_err;
				break;
				}
			default:
				{
				
				}
			}
		messg.source.data = lchn_name;
		messg.source.len = lenstr((char*)lchn_name);
		if (messg.source.len > UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SOURCE_MAX_LENGTH) messg.source.len = UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SOURCE_MAX_LENGTH;
			
			static uint8_t transfer_id = 0;
			
		unsigned long len_tx = uavcan_protocol_debug_LogMessage_encode(&messg, buffer);
		canardBroadcast (&g_canard,
										UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SIGNATURE,
										UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_ID,
										&transfer_id,
										CANARD_TRANSFER_PRIORITY_LOW ,
										buffer,
										len_tx);             //some indication
		}
	}

}





extern utimer_t TimerUAV_CubeNodeStatus;

void TUAVCAN::CB_FullRecvFrame (CanardInstance* ins, CanardRxTransfer* transfer)
{
	switch (transfer->data_type_id)
		{
		/*
		case UAVCAN_NODE_STATUS_DATA_TYPE_ID:
			{
			// содержимое сообщения не нужно, важен сам факт приема
			unsigned long val_nid;
			TEASYMEMSTORAGE::GetParam (ESAVEPARIX_UAVLINK_CONTROL_NID, &val_nid);
			if (transfer->source_node_id == val_nid) TimerUAV_CubeNodeStatus = 2100;
			//if (transfer->source_node_id == 10) ArmingMess_timer = 30000;
			break;
			}
		*/
		case UAVCAN_GET_NODE_INFO_DATA_TYPE_ID:			// обработка запроса, отправка resp
			{
			if ((transfer->transfer_type == CanardTransferTypeRequest)) getNodeInfoHandleCanard(transfer);
			break;
			}
		case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
			{
			getsetHandleCanard (transfer);
			break;
			}
		case UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_ID:
			{
			uavcan_protocol_param_ExecuteOpcodeRequest req_frm; 
			int32_t size_or_error = uavcan_protocol_param_ExecuteOpcodeRequest_decode (transfer, transfer->payload_len, &req_frm, 0);
				
			uavcan_protocol_param_ExecuteOpcodeResponse resp_frm;
			bool f_is_ok = true;
			if (size_or_error > 0)
				{
				switch (req_frm.opcode)
					{
					case UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_OPCODE_SAVE:
						{
						//extern void HF_test ();
						//HF_test ();  HardfaultTest for watchdog
						UAVSETS.Save_all ();
						break;
						}
					case UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_OPCODE_ERASE:
						{
						UAVSETS.Restore_all ();
						UAVSETS.Save_all ();
						break;
						}
					default:
						{
						f_is_ok = false;
						break;
						}
					}
				size_or_error++;
				}
			resp_frm.ok = f_is_ok;
			resp_frm.argument = 0;
				
			uint8_t rawtx[UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_MAX_SIZE];
			memset(rawtx, 0, UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_MAX_SIZE);
			size_or_error = uavcan_protocol_param_ExecuteOpcodeResponse_encode (&resp_frm, rawtx);
				int result = canardRequestOrRespond(&g_canard,
																		transfer->source_node_id,
																		UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE,
																		UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_ID,
																		&transfer->transfer_id,
																		transfer->priority,
																		CanardResponse,
																		&rawtx[0],
																		(uint16_t)size_or_error);
			
			break;
			}
		case UAVCAN_PROTOCOL_FILE_GETINFO_ID:				// обработка resp
			{
			FirmFileSize = UAVCAN_ParseGetInfo_resp (transfer);
			//transfer_id = transfer->transfer_id;
			if (FirmFileSize > 0) 
				{
				// info message
				ExternalFlagUpdate_Set ();
				TSTMSTRING str1((char*)&DebugMessBuf, sizeof(DebugMessBuf));
				//TBString str1((char*)&DebugMessBuf, sizeof(DebugMessBuf));
				//str1.Clear();
				str1 += "File size: ";
				str1 += FirmFileSize;
				str1 += " bytes.";
				MessageSend_info ((const char *)DebugMessBuf, str1.Length());
				// 
				FirmSw = 9;		// размер файла получен, очищаем в W25q16 место для прошивки и переходим на его прием (изменение SW в Firmware_SubTask ())
				//dbg_lastprocent = 0;
				}
			else
				{
				// если размер файла нулевой (то ждем повтора по таймауту)
				}
			break;
			}
		case UAVCAN_PROTOCOL_FILE_READ_ID:
			{
			if (read_file_transfer_id != transfer->transfer_id) break;
			if (TUAVCAN::GetNodeMode () != E_NDMODE_SOFTWARE_UPDATE) break;
			uavcan_protocol_file_ReadResponse resp_frame;
			unsigned char *lDarr = FirmBufData_RD;
			unsigned long Wr_offs = 0;
			long sz_recv = -1;
			unsigned char err = 0;
			bool f_end_is_ok = false;
			bool f_end_is_bad = false;
			//transfer_id = transfer->transfer_id;
			FirmSw = 11;
			int32_t size_or_error = uavcan_protocol_file_ReadResponse_decode (transfer, transfer->payload_len, &resp_frame, &lDarr);		// transfer->payload_len
			if (size_or_error >= 0)
				{
				if (resp_frame.error.value == UAVCAN_PROTOCOL_FILE_ERROR_OK)	// нет ошибок
					{
					if (resp_frame.data.len)
						{
						Wr_offs = FirmLoadByteOffs;
						FirmRepCount = C_UAVCAN_READFILE_REPEAT;
						//rx_firmware_amont += resp_frame.data.len;
						sz_recv = resp_frame.data.len;
						FirmLoadByteOffs += resp_frame.data.len;
						if (FirmLoadByteOffs >= FirmFileSize) 
							{
							f_end_is_ok = true;
							}
							
						WriteExtF_Plase (C_ADREXT_FIRMWARE_T, Wr_offs, resp_frame.data.data, sz_recv);
							
						// вывод сообщения downloading прогресса	
						unsigned long proc_const = FirmFileSize / 100;
						unsigned long proc_curent = FirmLoadByteOffs / proc_const;
						if (dbg_lastprocent != proc_curent)
							{
							unsigned char dlt = proc_curent - dbg_lastprocent;
							if (dlt >= 5)
								{
								if (proc_curent != 100)
									{
									TSTMSTRING str1((char*)&DebugMessBuf, sizeof(DebugMessBuf));
									str1 += proc_curent;
									str1 += " %";
									MessageSend_info ((const char *)DebugMessBuf, str1.Length());
									}
								dbg_lastprocent = proc_curent;
								}
							}
						fr_cnt++;
						}
					else
						{
						err++;
						}
					}
				else
					{
					err++;
					}
				}
			else
				{
				err++;
				}
			if (f_end_is_ok || f_end_is_bad)
				{
				FirmSw = 0;
				TUAVCAN::SetNodeMode ( E_NDMODE_OPERATIONAL);
				if (f_end_is_ok) 
					{
					// проверяем целостность принятого файла 
					bool rslt = ExternalFirmware_Check (*ExtFlash, C_ADREXT_FIRMWARE_T);
					if (rslt)
						{
						// создаем две копии firmware
						// стираем память
						EraseExtF_Place (C_ADREXT_FIRMWARE_A, FirmFileSize);
						EraseExtF_Place (C_ADREXT_FIRMWARE_B, FirmFileSize);
						// копируем удачно принятый файл
						ExtFlash->Copy (C_ADREXT_FIRMWARE_T, C_ADREXT_FIRMWARE_A, FirmFileSize);
						ExtFlash->Copy (C_ADREXT_FIRMWARE_T, C_ADREXT_FIRMWARE_B, FirmFileSize);
						// создаем флаг обновления
						ExternalFlagUpdate_Set ();
						}
					TSTMSTRING str1((char*)&DebugMessBuf, sizeof(DebugMessBuf));
					if (rslt)
						{
						str1 += ("100 % Complete ok.");
						}
					else
						{
						str1 += ("error file.");
						}
					MessageSend_info ((const char *)DebugMessBuf, str1.Length());
					}
				}
			FirmReadReq_timer = C_UAVCAN_RDFTECHWAIT_62MS;	// таймаут по спецификации uavcan
			break;
			}
		case UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_ID:
			{
			uavcan_protocol_file_BeginFirmwareUpdateRequest frame;
			unsigned char *lDarr = FirmWareName;
			unsigned char nod_id = 0;
			//transfer_id = transfer->transfer_id;
			int32_t ofs = uavcan_protocol_file_BeginFirmwareUpdateRequest_decode (transfer, transfer->payload_len, &frame, &lDarr);
			if (ofs && ofs < 32768)
				{
				if ((frame.image_file_remote_path.path.len < sizeof(FirmWareName)) && frame.image_file_remote_path.path.data)
					{
					// сохраняем имя/путь firmware
					FirmNameSize = frame.image_file_remote_path.path.len;
					memcpy ((void*)FirmWareName, (void*)frame.image_file_remote_path.path.data, frame.image_file_remote_path.path.len);
					}
				}
			nod_id = frame.source_node_id;
			if (!nod_id) nod_id = transfer->source_node_id;
			FirmNodeSrc = nod_id;
			// ответить
			uavcan_protocol_file_BeginFirmwareUpdateResponse ack;
			uint8_t buffer_resp[UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_OPTIONAL_ERROR_MESSAGE_MAX_LENGTH];
			memset(buffer_resp, 0, UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_OPTIONAL_ERROR_MESSAGE_MAX_LENGTH);
			memset(&ack, 0, sizeof(ack));
			if (TUAVCAN::GetNodeMode () != E_NDMODE_SOFTWARE_UPDATE)
				{
				ack.error = UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_OK;// UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_OK;
				f_NoInfo_FileSize = false;	// если команда getSizeFile не поддерживается(в mission planer), то установиться этот флаг, но чтение будет произведено
				TUAVCAN::SetNodeMode (E_NDMODE_SOFTWARE_UPDATE); // изменяем статус на статус прошивки
				FirmSw = 1;
				}
			else
				{
				ack.error = UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_RESPONSE_ERROR_OK; 
				}


				uint32_t len = uavcan_protocol_file_BeginFirmwareUpdateResponse_encode (&ack, buffer_resp);
				int result = canardRequestOrRespond(&g_canard,
																		transfer->source_node_id, /*transfer->source_node_id*/
																		UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_SIGNATURE,
																		UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_ID,
																		&transfer->transfer_id,
																		transfer->priority,
																		CanardResponse,
																		&buffer_resp[0],
																		(uint16_t)len);


			break;
			}
		case UAVCAN_PROTOCOL_RESTARTNODE_ID:
			{
			uavcan_protocol_RestartNodeRequest frame;
			uavcan_protocol_RestartNodeResponse resp;
			uavcan_protocol_RestartNodeRequest_decode (transfer, 0, &frame, 0);
			if (frame.magic_number == UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAGIC_NUMBER) 
				{
				resp.ok = true;
				TRESET::ResetToBoot (300);	// рестарт после того как gui tool примет ответ
				}
			else
				{
				resp.ok = false;
				}
			// отсылаем ответ
			unsigned char buf[UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAX_SIZE];
			uint32_t len = uavcan_protocol_RestartNodeResponse_encode (&resp, buf);
			if (len > 0)
				{
				int result = canardRequestOrRespond(&g_canard,
																		transfer->source_node_id,
																		UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE,
																		UAVCAN_PROTOCOL_RESTARTNODE_ID,
																		&transfer->transfer_id,
																		transfer->priority,
																		CanardResponse,
																		&buf[0],
																		(uint16_t)len);
				}
			break;
			}
		default:
			{
			CB_UserRecvFrame (ins, transfer);
			//f_test = true;
			break;
			}
		}
}



uint8_t TUAVCAN::GetVersion_MAJOR ()
{
	USERDATA *lInf = (USERDATA*)C_ADR_USERINFO;
	return (lInf->Version / 100);
}



uint8_t TUAVCAN::GetVersion_Minor ()
{
	USERDATA *lInf = (USERDATA*)C_ADR_USERINFO;
	return (lInf->Version % 100);
}


uint32_t TUAVCAN::GetProgram_CRC ()
{
	USERDATA *lInf = (USERDATA*)C_ADR_USERINFO;
	return lInf->CRC32_ENCODE;
}




bool TUAVCAN::ParseUAVCANCommandFromMaster ()
{
bool rv = false;
return rv;
}

