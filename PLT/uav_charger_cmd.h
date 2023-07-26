#ifndef _UAVCAN_SPAIBASE_CHARGER_CMD_BROADCAST_
#define _UAVCAN_SPAIBASE_CHARGER_CMD_BROADCAST_


#include <stdint.h>
#include "canard.h"



#define C_SPAISERIAL_LENGHT 5	

// команда для запросов на выдачу данных
#define UAVCAN_SPAIBASE_CMD_ID 20800
#define UAVCAN_SPAIBASE_CMD_NAME           "ardupilot.equipment.spaibase.SBaseCmd"
#define UAVCAN_SPAIBASE_CMD_SIGNATURE (0x6E2B53E858F1AD99ULL)


enum ESBATCMD {ESBATCMD_NONE = 0, ESBATCMD_GETSERIAL = 1, ESBATCMD_ON = 2, ESBATCMD_OFF = 3, ESBATCMD_GETBATDATA = 4, ESBATCMD_TIMESTAMP_A = 5, ESBATCMD_ENDENUM = 6};

/*
Команды:
	GetSerial
		Req: Запрос серийного номера (должно быть 1 устройство на шине, так как поле dst_id при этой команде не имеет смысла). Отсылка uav_equipment_spaibase_cmd_t, (поле param для зарядки = 'c', для борта = 'b')
		Resp: Отсылка uav_equipment_spaibase_cmd_t со всеми заполненными полями. В src_id будет искомый серийный номер устройства. (поле param == 0)
		
	Bat ON:
		Req: Отсылка uav_equipment_spaibase_cmd_t (отсылка с повторами >= 5 Hz). Включает батарею. (поле param для зарядки = 'c', для борта = 'b')
		Resp: -----
		
	Bat OFF:
		Req: Отсылка uav_equipment_spaibase_cmd_t (отсылка с повторами >= 5 Hz). Выключает батарею. (поле param для зарядки = 'c', для борта = 'b')
		Resp: -----
		
	GetFullBatData:
		Req: отсылка uav_equipment_spaibase_cmd_t. (поле param для зарядки = 'c', для борта = 'b')
		Resp: отсылка uav_equipment_sbattery_full_data_t. 

	SpaiTimestamp:
		Req: отсылка uav_equipment_spaibase_cmd_t. (поле param для зарядки = 'c', для борта = 'b')
		Resp: -----
*/

typedef struct
{
		// 7bit (1 = REQ, 0 - RESP), 6-0bit это код комманды:  1 - get serial req, 2 - on bat(multi), 3 - off(multi), 4 - get full bat data
		uint8_t cmd;						
		uint8_t param;
    uint8_t src_id[C_SPAISERIAL_LENGHT];
		uint8_t dst_id[C_SPAISERIAL_LENGHT];		// для комманд: 1,2, 3 поле неактуально
} uav_equipment_spaibase_cmd_t;





uint32_t uav_spaibase_cmd_encode (uav_equipment_spaibase_cmd_t *src, void* msg_buf, uint32_t offset);
int32_t uav_spaibase_cmd_decode (const CanardRxTransfer* transfer,  uav_equipment_spaibase_cmd_t *dest);



#define UAVCAN_SCHARGERBAT_DATA_ID 20801
#define UAVCAN_SCHARGERBAT_DATA_NAME           "ardupilot.equipment.spaibase.SBatData"
#define UAVCAN_SCHARGERBAT_DATA_SIGNATURE (0x4F4D20AAB581294BULL)
#define C_SBAT_VOLT_LSB 0.01F
#define C_SBAT_VOLTCELL_LSB ((6.0F / 255))
#define C_SBAT_CURRENT_LSB 0.01F
#define C_SBAT_CELL_MAXIMAL 16



typedef struct
{
		// 7bit в 1 это REQ, 6-0bit это код комманды:  0 - get serial req, 1 - on bat(multi), 2 - off(multi), 3 - get full bat data
		uint8_t cmd;			
		uint8_t param;
		uint8_t src_id[C_SPAISERIAL_LENGHT];         // id
    uint8_t dst_id[C_SPAISERIAL_LENGHT];         // id
		
		uint8_t bat_model;			// описывает особенности батареи (тип, напряжение ячеек ...)
		// по этим двум параметрам высчитывается номинальная емкость в вач/час
		uint16_t c_volt;				// напряжения полной зарядки для этой батареи 10 mv
		uint16_t c_current;			// номинальный ток баратеи в 10ma единицах

		// эти параметры показывают текущее напряжение батареи и текущий ток
		uint16_t now_volt;			// lsb = 10 mv  (то что видит батарея у себя)
		int16_t now_cur;				// lsb = 10 ma ((+) to bat, (-) from bat)  (то что видит батарея у себя)
		
		uint8_t cells_volt[C_SBAT_CELL_MAXIMAL];			// qunant: 6v / 255 = 0,02352941V lsb
	
		uint16_t cicles;							// history charge cicles  (циклом зарядки считается прием в акб номинального количества джоулей) WJ = V * A * 3600.
		uint16_t local_time_1s_lsb;		// время прошедшее с момента изменения статуса в 'ON' (в секундах). По существенной разнице этого параметра между другими АКБ можно диагностировать проблему
	
		int8_t temp;

} uav_equipment_sbattery_full_data_t;


uint32_t uav_equipment_sbattery_full_data_encode (uav_equipment_sbattery_full_data_t *src, void* msg_buf, uint32_t offset);
int32_t uav_equipment_sbattery_full_data_decode (const CanardRxTransfer* transfer, uav_equipment_sbattery_full_data_t *dest);




#endif
