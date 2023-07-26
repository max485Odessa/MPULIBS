#ifndef _H_TMAVLINK_STM32_H_
#define _H_TMAVLINK_STM32_H_


#include "main.h"
//#include "tusartdata.h"
#include "TFTASKIF.h"
#include "TUSARTRXB.h"
#include "SYSBIOS.H"
#include "rutine.h"



//#define C_MAXRXCIRCL_BUF_MAV 64
//#define C_MAX_TX_BUF_MAV 512
//#define C_HEARTBEAT_PERIOD 1000	// 1sek
//#define C_BATSTATUS_PERIOD 1000	// 1sek
//#define C_STATUSTEXT_TEST_PERIOD 3000	// 1sek
//#define C_MAVLINK_LINK_TIMEOUT 5000		// 5 sek
//#define C_VFR_ACTIVE_TIME 2000				// 
//#define C_GBLPOSINT_TIMEOUT 3000



#define C_MAVRXBUF_AMOUNT 2
#define C_MAVRXBUF_SIZE 2048

#ifdef M_PI
# undef M_PI
#endif
#define M_PI      (3.141592653589793f)

#ifdef M_PI_2
# undef M_PI_2
#endif
#define M_PI_2    (M_PI / 2)

#define M_GOLDEN  1.6180339f

#define M_2PI         (M_PI * 2)

#define DEG_TO_RAD      (M_PI / 180.0f)
#define RAD_TO_DEG      (180.0f / M_PI)




class TMAVLINKPRS : public TUSARTIFB, public TFFC{

	private:
		utimer_t LinkTimeout_period_usart;
	
		static utimer_t GlobPosTimeout_period;
		mavlink_global_position_int_t gposint_last;
		static float last_altitude_ground_level;
	
		uint8_t mav_rx_buf[C_MAVRXBUF_AMOUNT][C_MAVRXBUF_SIZE];
		unsigned char cur_isr_bufer_ix;
		unsigned short cur_isr_rx_ix;
		
		unsigned short GetRxData (uint8_t *lDst, unsigned short size_max);
	
		void Rx_Task ();
		bool MavlinkFrame_Rx (mavlink_message_t &m, mavlink_status_t &s);
		
		bool f_is_new_report_data;
		bool f_enable;
	
		bool f_new_adsb_frame;
		uavcan_equipment_adsb adsb_uav;
	
		void ConvADSB_Frame (mavlink_adsb_vehicle_t *lsrc, uavcan_equipment_adsb *ldst);
		
	
		virtual void ISR_DataRx (unsigned char dat);
	
	public:
		TMAVLINKPRS ();
	
		void Init (USART_TypeDef *p);
		virtual void Task ();
	
		bool GetLinkStatus ();
		void Enabled (bool val);
	
		uavcan_equipment_adsb *GetADSB ();
};




/*
DEC			HEX			COUNT			NAME
--------------------------------------------------------------------------------
33			21			1AD				GLOBAL POSITION
2				02			21F				SYSTEM TIME
163			A3			268				-
165			A5			27A				-
136			88			266				TERAIN REPORT
0				00			6D				HEART BEAT
193			C1			273				-
32			20			274				LOCAL POS NED
241			F1			284				VIBRATION
147			93			288				BATTERY STATUS
1				01			124				SYS STATUS
125			7D			1C4				POWER STATUS
152			98			1C7				-
62			3E			1C5				MAV CONTROLLER OUTPUT
42			2A			1CF				MISSION CURENT
36			24			1A2				SERVO OUNPUT RAW
65			41			2A1				RC CHANNELS
129			81			1C4				SCALLED IMU3
29			1D			1C4				SCALLED PRESSURE
137			89			1C7				SCALLER PRESSURE 2
150			96			2B				-
24			18			1C4				GPS RAW INT
30			1E			530				ATTITUDE
74			4A			8A2				VFR_HUD
178			B2			A22				-
182			B6			76A				-
27			1B			177				RAW IMU
116			74			18C				SCALLED IMU2
111			6F			15				TIME SYNC
22			16			3					PARAM VALUE
242			F2			21				HOME POSITION

*/


#endif

