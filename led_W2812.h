#ifndef _H_STM32_LED_W2812_H_
#define _H_STM32_LED_W2812_H_

#include "stm32f4xx_hal.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"
#include "hard_rut.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
extern void S_NOP ();
	 
#ifdef __cplusplus
}
#endif
	 
enum EBASECOLOR {ECOLR_BLACK = 0, ECOLR_RED = 1, ECOLR_GREEN = 2, ECOLR_BLUE = 3, ECOLR_YELOW = 4, ECOLR_ENDENUM};
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

#define C_LED_PORT GPIOB
#define C_LED_PIN GPIO_PIN_12

class TLEDWIF: public TFFC {

		unsigned char RGB_dat[3];				// �����
		bool f_update_need;							// ��� ������ ��� ���� ������ � ������
		utimer_t ResPeriodControl;
		void Wait_T (unsigned char val);
		void ByteColor_Tx (unsigned char colr);
		void Bit_Tx (bool val);
	
	public:
		TLEDWIF ();
		
		virtual void Task ();
		void Init ();
	
		void Set_RGB (uint8_t R, uint8_t G, uint8_t B);
		void Set_Color (EBASECOLOR enm_c);
		void Update ();
};



typedef struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
} s_rgb_t;


class TLED {
		float bright_mult;
	public:
		TLED ();
		uint8_t RGB_dat[3];
		void set_bright (uint8_t val);
		void color_rgb (uint8_t r, uint8_t g, uint8_t b);
		void color (EBASECOLOR eclr);
};



class TLEDS : public TFFC {
		
		void Bit_Tx (bool val);
		void Tx8bit (uint8_t colr);
		utimer_t relax_time;
		void update ();
		void update (TLED *ld);
	
	protected:
		const uint8_t *reixmx;
		const S_GPIOPIN *gp;
		bool f_need_update;
		virtual void Task () override;
		const uint8_t c_ar_cnt;
		TLED *array;
	
	public:
		TLEDS (const S_GPIOPIN *pn, uint8_t cnt, uint8_t *imx);
		void all_color (uint8_t r, uint8_t g, uint8_t b);
		void color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b);
		void all_bright (uint8_t val);
};



class TLEDEFFCT: public TLEDS {
		
		virtual void Task () override;
		
	public:
		TLEDEFFCT (const S_GPIOPIN *pn, uint8_t cnt, uint8_t *imx);
		void gen_progress (uint8_t proc, uint8_t lightmaxlevel, EBASECOLOR colb);
		void powdown_mode ();

};


#endif


