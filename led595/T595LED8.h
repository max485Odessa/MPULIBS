#ifndef _H_STM32_STDLIB_595LEDS8_H_
#define _H_STM32_STDLIB_595LEDS8_H_

#include "TFTASKIF.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "hard_rut.h"
#include "SYSBIOS.H"
#include "STMSTRING.h"



enum ELEDPIN {ELEDPIN_DIO = 0, ELEDPIN_SCK = 1, ELEDPIN_RCK = 2, ELEDPIN_ENDENUM = 3};
enum EALIGNDIG {EALIGNDIG_NONE, EALIGNDIG_LEFT, EALIGNDIG_MIDLE, EALIGNDIG_RIGHT, EALIGNDIG_ENDENUM};

#define SG_A 1
#define SG_B 2
#define SG_C 4
#define SG_D 8
#define SG_E 16
#define SG_F 32
#define SG_G 64
#define SG_P 128


class IDIG8 {
		TSTMSTRING *str;
		uint8_t *strdata;

		uint8_t get_celie_cnt (float val);
		void mark_big ();
		
	
	protected:
		uint32_t dig_cnt;
		EALIGNDIG align_mode;
		uint8_t *rawdata;
	
		void clear ();
		void str_to_ledbuf (uint8_t ix_dst);
		virtual void write_data (uint8_t ix, uint8_t dat) = 0;
		virtual uint8_t read_data (uint8_t ix) = 0;
	
	public:
		IDIG8 (uint8_t dgcnt);
		void draw_raw (uint8_t ix, uint8_t dat);
		void draw_raw (char *str);
		void draw_long (long val);
		void draw_float (float val, uint8_t dr_cnt);
		void set_align (EALIGNDIG a_md);
};



class TLED595: public IDIG8, public TFFC {
	private:
		
		S_GPIOPIN *pins;
		bool f_need_strobe;
		uint32_t regen_ix;
		uint32_t delay_ms;
		utimer_t update_timer;
		virtual void Task ();
		void clear_ledbuf ();
		static void regen_task_1ms_hard (void *obj);
		void regen_task_1ms ();
		void strobe_rclk ();
		void strobe_sclk ();
		void send_byte (uint8_t data);
		uint8_t dign_to_bit (uint8_t n);
	
		virtual void write_data (uint8_t ix, uint8_t dat);
		virtual uint8_t read_data (uint8_t ix);
	protected:
	public:
		
		TLED595 (S_GPIOPIN *pns, uint8_t dgcnt);
		
				
};


#endif
