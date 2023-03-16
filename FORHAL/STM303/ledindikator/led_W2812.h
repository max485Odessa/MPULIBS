#ifndef _H_STM32_LED_W2812_H_
#define _H_STM32_LED_W2812_H_

#include "resources.h"
#include "stm32l4xx_hal.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"
#include "hard_rut.h"


enum EBASECOLOR {ECOLR_NONE = 0, ECOLR_BLACK = 1, ECOLR_RED = 2, ECOLR_GREEN = 3, ECOLR_BLUE = 4};
enum EWAITNANO {EWN_1T = 0, EWN_2T = 1};
const unsigned char C_MAXCOLOR_LEVEL = 0xFF;
const utimer_t C_W2818_TRESETPERIOD = 3;		// 3 ms ������ ����������

//#define C_WLEDPIN GPIO_PIN_0
//const uint16_t C_WLEDPIN = GPIO_PIN_0; 


/*
1.25 ������������ ������ 1 ����

���������� ��������� ���������� ���������� �� ������ 80 ����������� � ������� ������������ ������� ���� �������.
��� ����������� ������, ������������� ����������� ���������� ������ ���������� ������������� ��������� C_W2818_TRESETPERIOD.
���� ������ ������� ��������� ����� ���������� ���������� ������� ������� 3 ms, �� ���������� ����������� �������� ��������� � ����� �������
 � C_W2818_TRESETPERIOD. � ����� ������ ����� �������� C_W2818_TRESETPERIOD ��������� ����� ��������.
*/


#define C_LEDSTROBE_FRAME_TIME 2		// ������ ���� ������ ��� ����� 2

class TLEDWIF: public TFFC {

		unsigned char RGB_dat[3];				// �����
		bool f_update_need;							// ��� ������ ��� ���� ������ � ������
		utimer_t ResPeriodControl;
		void Wait_T (unsigned char val);
		void ByteColor_Tx (unsigned char colr);
		void Bit_Tx (bool val);
	
	public:
		TLEDWIF ();
		void Init ();
		virtual void Task ();
	
		void Set_RGB (uint8_t R, uint8_t G, uint8_t B);
		void Set_Color (EBASECOLOR enm_c);
		void Update ();
};



class TLED {
	public:
		TLED ();
		uint8_t RGB_dat[3];
		void color_rgb (uint8_t r, uint8_t g, uint8_t b);
		void color (EBASECOLOR eclr);
};



class TLEDS : public TFFC {
		TLED *array;
		const uint8_t c_ar_cnt;
		const S_GPIOPIN *gp;
		virtual void Task ();
		
		void Bit_Tx (bool val);
		void Tx8bit (uint8_t colr);
		utimer_t relax_time;
		bool f_need_update;
		void update ();
		void update (TLED *ld);
	
	public:
		TLEDS (const S_GPIOPIN *pn, uint8_t cnt);
		void all_color (uint8_t r, uint8_t g, uint8_t b);
		void color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b);
};


#endif


