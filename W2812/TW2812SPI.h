#ifndef _H_W2812_SPI_VARIANT_H_
#define _H_W2812_SPI_VARIANT_H_



#include "hard_rut.h"
#include "TEXECTASK.hpp"
#include "SYSBIOS.H"



// 42 Mhz in witch prescaler == 2



class TW2812LEDS: public TFRDEXEC {
	public:
		typedef struct {
			ESYSSPI p_ix;
			uint8_t af;
			S_GPIOPIN mosi;
			} S_W2812MOSI_T;
#pragma pack (push,1)
		typedef struct {
			union {
				struct {
					uint8_t g;
					uint8_t r;
					uint8_t b;
					};
				uint8_t arr[3];
				}u;
		} S_GRB_T;
#pragma pack (pop)
		
		TW2812LEDS (S_W2812MOSI_T m, uint16_t w, uint16_t h);
		TW2812LEDS (S_W2812MOSI_T m, uint32_t cnt);
			
		void enable (bool v);
		void set_update_period (uint16_t ms);
			
	private:
		SPI_HandleTypeDef SpiHandle;
		SYSBIOS::Timer updperiod_timer;
	
		const uint16_t c_width;
		const uint16_t c_height;
		const uint32_t c_leds_amount;

		uint16_t c_update_period;
	
		uint16_t raw_size ();
		S_W2812MOSI_T pinmosi;
		uint8_t *rawdmaarray;
		S_GRB_T *canva;
		bool f_enable;
	
		void canva_to_raw ();
	
		void init_spi ();
		void start_frame ();
		bool f_need_update;

		void hardconstruct ();
		void task () override;

};



#endif