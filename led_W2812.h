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
	 
enum EBASECOLOR {ECOLR_BLACK = 0, ECOLR_RED = 1, ECOLR_GREEN = 2, ECOLR_BLUE = 3, ECOLR_YELOW = 4, ECOLR_WHITE = 5, ECOLR_ENDENUM};
enum EWAITNANO {EWN_1T = 0, EWN_2T = 1};
const unsigned char C_MAXCOLOR_LEVEL = 0xFF;
const utimer_t C_W2818_TRESETPERIOD = 3;		// 3 ms период обновления


#define C_LEDSTROBE_FRAME_TIME 50		// ms
#define C_LED_PORT GPIOB
#define C_LED_PIN GPIO_PIN_12

#pragma pack (push,4)

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
		void set_bright (float val);
		void color_rgb (uint8_t r, uint8_t g, uint8_t b);
		void color_rgb (s_rgb_t &c);
		void color_rgb (s_rgb_t *lc);
		void color (EBASECOLOR eclr);
		static void color (EBASECOLOR eclr, s_rgb_t &dst, float brlv);
};


//enum ELEDSWISR {ELEDSWISR_SYNC = 0, ELEDSWISR_LEDS, ELEDSWISR_WAIT, ELEDSWISR_ENDENUN};
class TLEDS : public TFFC {		//  
		
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
		TLED **array;
	
		bool f_isr_sync;
	
	public:
		TLEDS (const S_GPIOPIN *pn, uint8_t cnt, uint8_t *imx);
		void all_color (uint8_t r, uint8_t g, uint8_t b);
		void all_color (const s_rgb_t &c);
		void color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b);
		void color (uint8_t ix, const s_rgb_t &c);
		void all_bright (float val);
		void bright (uint8_t ix, float val);
		uint8_t leds_cnt ();
	
		TLED *l10 (uint8_t ix);
	
		void sync_after_isr ();
	

};





#pragma pack (pop)


#endif


