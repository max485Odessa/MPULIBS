#ifndef _H_RAWCAN_MODULE_H_
#define _H_RAWCAN_MODULE_H_



#include "hard_rut.hpp"
#include "rutine.hpp"
#include "TTFIFO.hpp"
#include "SYSBIOS.hpp"
#include "THIZIF.hpp"
#include "stm32f4xx_hal_can.h"

enum EAK70SW {EAK70SW_NONE = 0,  EAK70SW_GETSTATE, EAK70SW_INIT, EAK70SW_INIT_B, EAK70SW_EXIT, EAK70SW_STOP_0, EAK70SW_START_0, EAK70SW_LEFT, EAK70SW_STOP_1, EAK70SW_START_1, EAK70SW_RIGHT, EAK70SW_STOP_2 , \
EAK70SW_START_RIGHT_SPEED_A, EAK70SW_START_RIGHT_SPEED_B, EAK70SW_PASIVE, \
EAK70SW_SERVOTEST, \
EAK70SW_SERV_RUN, \
EAK70SW_ENDENUM};



typedef struct
{
  uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                        will be received. This parameter can be a value of
                        @ref CAN_identifier_type */

  uint8_t RTR;     /*!< Specifies the type of frame for the received message.
                        This parameter can be a value of
                        @ref CAN_remote_transmission_request */

  uint8_t DLC;     /*!< Specifies the length of the frame that will be received.
                        This parameter can be a value between 0 to 8 */

  uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to
                        0xFF. */

  uint8_t FMI;     /*!< Specifies the index of the filter the message stored in
                        the mailbox passes through. This parameter can be a
                        value between 0 to 0xFF */
} CanRxMsg;


typedef struct
{
  uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                        will be transmitted. This parameter can be a value
                        of @ref CAN_identifier_type */

  uint8_t RTR;     /*!< Specifies the type of frame for the message that will
                        be transmitted. This parameter can be a value of
                        @ref CAN_remote_transmission_request */

  uint8_t DLC;     /*!< Specifies the length of the frame that will be
                        transmitted. This parameter can be a value between
                        0 to 8 */

  uint8_t Data[8]; /*!< Contains the data to be transmitted. It ranges from 0
                        to 0xFF. */
} CanTxMsg;



typedef struct
{
  uint16_t CAN_FilterIdHigh;         /*!< Specifies the filter identification number (MSBs for a 32-bit
                                              configuration, first one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterIdLow;          /*!< Specifies the filter identification number (LSBs for a 32-bit
                                              configuration, second one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterMaskIdHigh;     /*!< Specifies the filter mask number or identification number,
                                              according to the mode (MSBs for a 32-bit configuration,
                                              first one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterMaskIdLow;      /*!< Specifies the filter mask number or identification number,
                                              according to the mode (LSBs for a 32-bit configuration,
                                              second one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterFIFOAssignment; /*!< Specifies the FIFO (0 or 1) which will be assigned to the filter.
                                              This parameter can be a value of @ref CAN_filter_FIFO */

  uint8_t CAN_FilterNumber;          /*!< Specifies the filter which will be initialized. It ranges from 0 to 13. */

  uint8_t CAN_FilterMode;            /*!< Specifies the filter mode to be initialized.
                                              This parameter can be a value of @ref CAN_filter_mode */

  uint8_t CAN_FilterScale;           /*!< Specifies the filter scale.
                                              This parameter can be a value of @ref CAN_filter_scale */

  FunctionalState CAN_FilterActivation; /*!< Enable or disable the filter.
                                              This parameter can be set either to ENABLE or DISABLE. */
} CAN_FilterInitTypeDef;

// ��������� ������ � ��������: -12.5  - 12.5 ������, ���  -716,1972  � 716,1972 ��������� ���� � �� ���������� ��������� ���� ������� -95.5 : 95.5 (191 ��������)
// 716,1972/95.5(95) = 7,49944712(������� �� �����)
// 360 / 7,49944712 =  48.00353869  (������� ������� � 360 ��������)
// 3,819718888888888 -  ������ ������� (360 ��������) �� �������� 191



enum EMTRDIRMOV {EMTRDIRMOV_NONE = 0, EMTRDIRMOV_UP = 1/*�� �������*/, EMTRDIRMOV_DOWN = 2/*������ �������*/, EMTRDIRMOV_ENDENUM = 3};

typedef struct {
	float metter;							// ������ ��������� �������
	unsigned char last_dir;		// enum EMTRDIRMOV
	unsigned char is_move;		// 0 - stoped
	unsigned char fps;
} S_MYPARAMS_T;

#define CAN_TxStatus_Failed         ((uint8_t)0x00)/*!< CAN transmission failed */
#define CAN_TxStatus_Ok             ((uint8_t)0x01) /*!< CAN transmission succeeded */
#define CAN_TxStatus_Pending        ((uint8_t)0x02) /*!< CAN transmission pending */
#define CAN_TxStatus_NoMailBox      ((uint8_t)0x04) /*!< CAN cell did not provide*/

#define CAN_FIFO0                 ((uint8_t)0x00) /*!< CAN FIFO 0 used to receive */
#define CAN_FIFO1                 ((uint8_t)0x01) /*!< CAN FIFO 1 used to receive */
#define CAN_Id_Standard             ((uint32_t)0x00000000)  /*!< Standard Id */
#define CAN_Id_Extended             ((uint32_t)0x00000004)  /*!< Extended Id */
#define IS_CAN_IDTYPE(IDTYPE) (((IDTYPE) == CAN_Id_Standard) || \
                               ((IDTYPE) == CAN_Id_Extended))

/* CAN Mailbox Transmit Request */
#define TMIDxR_TXRQ       ((uint32_t)0x00000001) /* Transmit mailbox request */

#define C_RPQ_AK70 ((float)48.00353869)  // QuantRotatePer
#define C_QUANT_AK70_POLE ((float)95.5)
#define C_QUANT_AK70_DIAP (C_QUANT_AK70_POLE+C_QUANT_AK70_POLE) // 191
#define C_PI ((float)3.14159265359)
#define C_COIL_DIAMETER ((float)16.0)
	
enum EAKMODE {EAKMODE_NONE = 0, EAKMODE_AUTO, EAKMODE_MANUAL, EAKMODE_ENDENUM};
enum EACCL {EACCL_NOME = 0, EACCL_INC, EACCL_DEC, EACCL_ENDENUM};

class TAK70 {

		uint8_t ubKeyNumber = 0x0;

		EAK70SW sw;
		EAK70SW sw_next;
		utimer_t accell_timer;
		utimer_t tx_period_timer;
		utimer_t status_period_timer;
		utimer_t mydata_period_timer;
		bool f_status_que;
		CAN_HandleTypeDef can1;

		CanTxMsg TxMsg;		// TxMessage
	
		void send_cmd_read_status ();
		void send_cmd_unlock ();
		void send_cmd_exit ();
		void send_cmd_zeropos ();
		void send_my_params ();
		void set_stdid (unsigned short adr);
		unsigned short std_id;
	
		float cur_cur;
		float cur_speed;
		float cur_pos;
		float pos_add_val;
		
		void send_current_motor_data ();		// float spd, float cur
		//void send_current_servo_data ();	
		
		long float_to_uint(float x, float x_min, float x_max, unsigned int bits);
		float uint_to_float(int x_int, float x_min, float x_max, int bits);
		void unpack_reply (const CanRxMsg &msg, float &postion, float &speed, float &torque);
		
		float state_speed;
		float state_pos;
		float state_current;
		unsigned long last_timestamp_rx_state;
		bool f_new_motor_state;
		bool f_motor_link_state;
		

		void SetCurrent (float val);
		void SetSpeed (float val);
		void SetKD (unsigned short val);
		void SetKP (unsigned short val);
		void SetPosition (float val);
		
		
		EMTRDIRMOV detect_dir;
		EMTRDIRMOV last_detect_dir;
		float RotateCounterAK;
		float CoilMetter;
		float conv_quant_191 (float val);
		float prev_quant_pos;
		void movement_task (float next_pos);
		float get_delta_quant ();
		EMTRDIRMOV get_move_direction ();
		unsigned char cntr_fps;
		unsigned char cntr_fps_acc;
		utimer_t last_fps_time;
	
		float get_wire_external_lenght ();
		
		float manual_point_pos;
		float manual_point_metter;
		EAKMODE mode;
		void Task_auto ();
		void Task_Manual ();
		EMTRDIRMOV key_state;
		EACCL accel_state;
		float c_accel_max;
		float c_accel_incdec_val;
		void accell_task ();
		
		bool f_pos_back_run;
		void pos_back_task ();


		const S_GPIOPIN *power_pin;
		bool is_poweres_state;
	
	public:
		TAK70 (const S_GPIOPIN *pin, bool f_state);
		void Init ();
		void Task ();
		void Power_enabled (bool val);


		void SetMaxCurrent (float curval);
		void SetWorkCurrent (float val);
		void SetMinCurent (float val);
		void SetCurrentSleepTime (float val);							// ���������: ����� ���� �������� ������� �������� ���� ������ ������������ ����� ������� � ������������� ����������� ��� (�� SetCurrentMinimal )	
		void SetSpeedAccell_down (float val);							// ���������: ����� ������ ������������ �������� (������ ��������� � SetSpeedMax_down)
		void SetSpeedMax_down (float val);								// ���������: ������������ �������� (������ ��������� � SetSpeedAccell_down)
	
		void KeyPush (EMTRDIRMOV ckcod);

		void SetMode (EAKMODE  mdd);
		EAKMODE GetMode ();
	
		float GetSpeed ();
		float GetPos ();		// 0-359 ����
		float GetCurrent ();
		bool GetLink ();

};



#endif

