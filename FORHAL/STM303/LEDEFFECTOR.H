#ifndef _H_W2812_EFFECTOR_CLASS_H_
#define _H_W2812_EFFECTOR_CLASS_H_

#include "resources.h"
#include "stm32l4xx_hal.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"
#include "rutine.h"


extern void disable_irq ();
extern void enable_irq ();

enum ELEFCT {ELEFCT_NONE = 0, ELEFCT_A, ELEFCT_B, ELEFCT_C, ELEFCT_D, ELEFCT_E, ELEFCT_ENDENUM = 6};
enum ELPROGSTATE {ELPROGSTATE_NONE = 0, ELPROGSTATE_PLAY, ELPROGSTATE_PAUSE, ELPROGSTATE_ENDENUM = 3};
enum ELCOLOR {ELCOLOR_BLACK = 0, ELCOLOR_RED, ELCOLOR_GREEN, ELCOLOR_BLUE, ELCOLOR_YELLOW, ELCOLOR_WHITE, ELCOLOR_ENDENUM = 6};

#define C_LEDS_AMOUNT 2
#define C_LEDS_PCBLINE 26

#pragma pack(push,1)


typedef struct {
	long val;
	float step;
} SFUKRTG;


typedef struct {
	unsigned char cnt;
	SFUKRTG *array;
} SFLAGUKRTG;


typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} SLEDSD;


typedef struct {
	SLEDSD rgb[C_LEDS_AMOUNT];
	unsigned short TimeTransition;
	unsigned short TimeWaiting;
} SLEDSCENE;


typedef struct {
	unsigned short scene_count;
	SLEDSCENE *scene;
} SLEDPROGRAM;


typedef struct {
	float r;
	float g;
	float b;
} SLEDSDF;


typedef struct {
	SLEDSDF rgb[C_LEDS_AMOUNT];
	SLEDSDF step[C_LEDS_AMOUNT];
	unsigned short TimeTransition;
	unsigned short TimeWaiting;
} SLGRADIENTSTATE;



typedef struct {
	ELPROGSTATE prog_state;
	SLEDPROGRAM *lp_prog;						// указатель на текущюю программу
	
	SLGRADIENTSTATE gradient_state;	// текущее состояние перехода градиента между текущей сценой и следующей
	unsigned short scene_ix;				// текущий индекс сцены
	
} SLDPROGRAMM;


#pragma pack(pop)


class TLEDECECT: public TFFC {
	
		ELEFCT sw;
		utimer_t PeriodControl;
		utimer_t NextProgramTimer;
		
		SLDPROGRAMM state;
	
		void start_prog (const SLEDPROGRAM *prog);
		void stop ();
	
		void scene_set (unsigned short scnix);
		void scene_next ();
		void scene_gradient_update_10ms ();
	
		unsigned short next_scene_ix (unsigned short scn_ix);
	
		void calculate_scene_gradient (SLEDSCENE *cur_scn, SLEDSCENE *next_scn);
		
		void update_led_phisical ();
		void bit_tx_a (bool val);
		void bit_tx_b (bool val);
		void byte_color_tx_a (unsigned char colr);
		void byte_color_tx_b (unsigned char colr);
		
		void phisical_full_line_a (SLEDSDF *rgb);
		void phisical_full_line_b (SLEDSDF *rgb);
		
		void phisical_one_led_update_a (unsigned char buf[3]);
		void phisical_one_led_update_b (unsigned char buf[3]);

		void next_program_start ();
		
		
		SLEDSD leds_arr_a[C_LEDS_PCBLINE];
		SLEDSD leds_arr_b[C_LEDS_PCBLINE];
		void phisical_many_led_update_a (SLEDSD leds[C_LEDS_PCBLINE]);
		void phisical_many_led_update_b (SLEDSD leds[C_LEDS_PCBLINE]);
		void phisical_many_leds_update ();
		
		void clear_array_a ();
		void clear_array_b ();
		void clear_arrays ();
		
		void point_set (SLEDSD *dst, unsigned char point_ix, unsigned char size, ELCOLOR fone_c);
		void line_fill (SLEDSD *dst, unsigned char p_ix_start, unsigned char p_ix_stop, ELCOLOR fone_c);
		
		void line_blur (SLEDSD *dst, unsigned char p_ix_start, unsigned char p_ix_stop);
		void color_mux (SLEDSD *in1, SLEDSD *in2, SLEDSD *out);
		
		float c_effect_var_depth;
		float effect_var;
		float effect_var_step;
		unsigned char stl_ix;
		void effect_ukraine ();
		
		
		float effect_point_pos;
		float effect_point_pos_step;
		void effect_fly (bool val);
		
	
	public:
		
		TLEDECECT ();
		void Init ();
		virtual void Task ();
		void StartProgramm (ELEFCT esw);
};


#endif

