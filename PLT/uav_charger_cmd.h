#ifndef _UAVCAN_SPAIBASE_CHARGER_CMD_BROADCAST_
#define _UAVCAN_SPAIBASE_CHARGER_CMD_BROADCAST_


#include <stdint.h>
#include "canard.h"



#define C_SPAISERIAL_LENGHT 5	

// ������� ��� �������� �� ������ ������
#define UAVCAN_SPAIBASE_CMD_ID 20800
#define UAVCAN_SPAIBASE_CMD_NAME           "ardupilot.equipment.spaibase.SBaseCmd"
#define UAVCAN_SPAIBASE_CMD_SIGNATURE (0x6E2B53E858F1AD99ULL)


enum ESBATCMD {ESBATCMD_NONE = 0, ESBATCMD_GETSERIAL = 1, ESBATCMD_ON = 2, ESBATCMD_OFF = 3, ESBATCMD_GETBATDATA = 4, ESBATCMD_TIMESTAMP_A = 5, ESBATCMD_ENDENUM = 6};

/*
�������:
	GetSerial
		Req: ������ ��������� ������ (������ ���� 1 ���������� �� ����, ��� ��� ���� dst_id ��� ���� ������� �� ����� ������). ������� uav_equipment_spaibase_cmd_t, (���� param ��� ������� = 'c', ��� ����� = 'b')
		Resp: ������� uav_equipment_spaibase_cmd_t �� ����� ������������ ������. � src_id ����� ������� �������� ����� ����������. (���� param == 0)
		
	Bat ON:
		Req: ������� uav_equipment_spaibase_cmd_t (������� � ��������� >= 5 Hz). �������� �������. (���� param ��� ������� = 'c', ��� ����� = 'b')
		Resp: -----
		
	Bat OFF:
		Req: ������� uav_equipment_spaibase_cmd_t (������� � ��������� >= 5 Hz). ��������� �������. (���� param ��� ������� = 'c', ��� ����� = 'b')
		Resp: -----
		
	GetFullBatData:
		Req: ������� uav_equipment_spaibase_cmd_t. (���� param ��� ������� = 'c', ��� ����� = 'b')
		Resp: ������� uav_equipment_sbattery_full_data_t. 

	SpaiTimestamp:
		Req: ������� uav_equipment_spaibase_cmd_t. (���� param ��� ������� = 'c', ��� ����� = 'b')
		Resp: -----
*/

typedef struct
{
		// 7bit (1 = REQ, 0 - RESP), 6-0bit ��� ��� ��������:  1 - get serial req, 2 - on bat(multi), 3 - off(multi), 4 - get full bat data
		uint8_t cmd;						
		uint8_t param;
    uint8_t src_id[C_SPAISERIAL_LENGHT];
		uint8_t dst_id[C_SPAISERIAL_LENGHT];		// ��� �������: 1,2, 3 ���� �����������
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
		// 7bit � 1 ��� REQ, 6-0bit ��� ��� ��������:  0 - get serial req, 1 - on bat(multi), 2 - off(multi), 3 - get full bat data
		uint8_t cmd;			
		uint8_t param;
		uint8_t src_id[C_SPAISERIAL_LENGHT];         // id
    uint8_t dst_id[C_SPAISERIAL_LENGHT];         // id
		
		uint8_t bat_model;			// ��������� ����������� ������� (���, ���������� ����� ...)
		// �� ���� ���� ���������� ������������� ����������� ������� � ���/���
		uint16_t c_volt;				// ���������� ������ ������� ��� ���� ������� 10 mv
		uint16_t c_current;			// ����������� ��� ������� � 10ma ��������

		// ��� ��������� ���������� ������� ���������� ������� � ������� ���
		uint16_t now_volt;			// lsb = 10 mv  (�� ��� ����� ������� � ����)
		int16_t now_cur;				// lsb = 10 ma ((+) to bat, (-) from bat)  (�� ��� ����� ������� � ����)
		
		uint8_t cells_volt[C_SBAT_CELL_MAXIMAL];			// qunant: 6v / 255 = 0,02352941V lsb
	
		uint16_t cicles;							// history charge cicles  (������ ������� ��������� ����� � ��� ������������ ���������� �������) WJ = V * A * 3600.
		uint16_t local_time_1s_lsb;		// ����� ��������� � ������� ��������� ������� � 'ON' (� ��������). �� ������������ ������� ����� ��������� ����� ������� ��� ����� ��������������� ��������
	
		int8_t temp;

} uav_equipment_sbattery_full_data_t;


uint32_t uav_equipment_sbattery_full_data_encode (uav_equipment_sbattery_full_data_t *src, void* msg_buf, uint32_t offset);
int32_t uav_equipment_sbattery_full_data_decode (const CanardRxTransfer* transfer, uav_equipment_sbattery_full_data_t *dest);




#endif
