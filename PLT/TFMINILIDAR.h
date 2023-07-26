#ifndef _H_TUSARTLIDAR_STM32_H_
#define _H_TUSARTLIDAR_STM32_H_

#include "TUSARTRXB.h"
#include "TFTASKIF.h"
#include "rutine.h"
#include "main.h"


/*
typedef __packed struct {
	unsigned char Atr1;
	unsigned char Atr2;
	unsigned char Dist_L;
	unsigned char Dist_H;
	unsigned char Strench_L;
	unsigned char Strench_H;
	unsigned char Mode;
	unsigned char Spare;
	char Crc8;
} SLIDATFRAME;
*/


typedef __packed struct {
	unsigned char Atr1;
	unsigned char Atr2;
	unsigned char Dist_L;
	unsigned char Dist_H;
	unsigned char Strench_L;
	unsigned char Strench_H;
	unsigned char Temp_L;
	unsigned char Temp_H;
	char Crc8;
} SLIDATFRAME;



typedef struct {
	float range;
	float min_dist;
	float max_dist;
} LIDOUTDATA;



enum ELIDARMEASTYPE {ELIDARMEASTYPE_SHORT_0 = 0, ELIDARMEASTYPE_SHORT_2 = 2, ELIDARMEASTYPE_MIDLE = 3, ELIDARMEASTYPE_LONG = 7};


typedef __packed struct {
	SLIDATFRAME slot;		// сам фрейм данных
	bool full;
} LIDARBUF;


#define C_USART_LIDAR_PORT USART1
#define C_LIDAR_INDX_PORT 1
#define C_LIDAR_BF_SIZE 2048
#define C_LIDAR_FRAMESYNCATTR_BYTE 0x59
#define C_LIDARBUF_AMOUNT 2
#define C_RECVBUF_TAILSIZE 1
#define C_LIDARRECVBUF_SIZE (sizeof(SLIDATFRAME) + C_RECVBUF_TAILSIZE)	// размер пакета + 1 возможный байт на прошлое crc со значением 0x59
#define C_DEF_LIDAR_RATE 1000

class TUSARTLIDAR : public TUSARTIFB, public TFFC {
	private:
		utimer_t timer_rate;
		unsigned long c_rate_period;
		LIDOUTDATA cur_lidar_data;
		bool f_new_lidar_data_uav;
		bool f_new_lidar_data_feed;
	
		static bool f_new_lidar_distanse;
		static float curent_metter;
	
		unsigned char sw;
		bool f_sync;
	
		void clear_recv_buf ();
		unsigned char recv_buf[C_LIDARRECVBUF_SIZE];
		unsigned char recv_b_ix;				// указатель на заполняемый буфер
		unsigned char recv_atr_ix;			// указатель на заполняемый буфер
		unsigned char recv_sync;
		
		char LidarCRC (void *ldat, unsigned char size);

		LIDARBUF frame[C_LIDARBUF_AMOUNT];
		char frame_tx_indx;							// указатель на заполняемый кадр
		char frame_rx_indx;							// указатель на извлекаемый кадр
		
		bool PushFrame (SLIDATFRAME *ldat);
		bool PopFrame (SLIDATFRAME *ldat);
	
		SLIDATFRAME *CheckLidarData ();					// проверяет принятые данные (crc и тд) и возвращает начальный адрес размещения фрейма
		void ClearSlot (char ix);

		
		#ifdef DEBUG_LIDAR
		unsigned long lidar_good_frame_cnt;
		unsigned long lidar_alignment_frame_cnt;
		unsigned long lidar_bad_frame_cnt;
		unsigned long lidar_bad_crc_frame_cnt;
		unsigned long lidar_bad_many_sync_frame_cnt;
		unsigned long lidar_bad_over_frame_cnt;
		#endif
	
	
	protected:
		
		//USART_InitTypeDef USART_InitStructure;
	
		BUFPAR rx_buf;
	
		virtual void ISR_DataRx (unsigned char dat);
		void ClearRxBufers ();
	
		bool f_enable;
		
	public:
		TUSARTLIDAR ();

		void Init (BUFPAR *rx_b, BUFPAR *tx_b);
		bool CheckFrame ();
		virtual void Task ();
	
		bool GetLidarData_feed (LIDOUTDATA *lout);
		bool GetLidarData_uav (LIDOUTDATA *lout);
		void SetRate_uav (float hz_val);
		void SetEnabled (bool val);
		static bool GetDistance (float &value);
	
};




#endif
