#include "LEDEFFECTOR.H"

#define C_NEXT_STARTLED_TIME 10000
#define C_SCEN_A_COUNT 16
#define C_MVD_BLINK_TIME 4
#define C_HEALTH_BLINK_TIME 12
#define C_LIGHT_DIVIDER 1

#define C_UKRANIAN_EF_SIZE 3.0
#define C_UKRANIAN_EF_STEP 0.3


#define C_FLY_EF_POINTIX 0
#define C_FLY_EF_STEP 0.5
#define C_FLY_EF_PIXSIZE 3


#define C_UKRSHAT_AMOUNT 10
static const SFUKRTG stlarr[C_UKRSHAT_AMOUNT] = {{1,0.1}, {2,0.1}, {2,0.1}, {2,0.1}, {3,0.2}, {3,0.2}, {2,0.1}, {2,0.05}, {1,0.2}, {2,0.25}};
static const SFLAGUKRTG urkshatal = {C_UKRSHAT_AMOUNT, (SFUKRTG*)stlarr };


static const SLEDSD color_code[ELCOLOR_ENDENUM] = { {0,0,0}, {255,0,0}, {0,255,0}, {0,0,255}, {255,255,0}, {255,255,255}};	// rgb определение некоторых цветов


static const SLEDSCENE scens_a[C_SCEN_A_COUNT] = {
{255,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{255,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{255,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{255,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{0,0,0, 0,0,255, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{0,0,0, 0,0,255, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{0,0,0, 0,0,255, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
{0,0,0, 0,0,255, 0, C_MVD_BLINK_TIME}, {0,0,0, 0,0,0, 0, C_MVD_BLINK_TIME}, \
};
static const SLEDPROGRAM prog_a = {C_SCEN_A_COUNT, (SLEDSCENE*)scens_a};



#define C_SCEN_B_COUNT 12
static const SLEDSCENE scens_b[C_SCEN_B_COUNT] = {
{255,255,255, 0,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
{255,255,255, 0,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
{255,255,255, 0,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
{0,0,0, 255,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
{0,0,0, 255,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
{0,0,0, 255,0,0, C_HEALTH_BLINK_TIME,0}, {0,0,0, 0,0,0, C_HEALTH_BLINK_TIME,0}, \
};
static const SLEDPROGRAM prog_b = {C_SCEN_B_COUNT, (SLEDSCENE*)scens_b};


//#define C_SCEN_C_COUNT 5
//static const SLEDSCENE scens_c[C_SCEN_C_COUNT] = {{255,0,255, 0,0,255, 0,0}, {255,0,255, 0,0,255, 0,0}, {255,0,255, 0,0,255, 0,0}, {255,0,255, 0,0,255, 0,0}, {255,0,255, 0,0,255, 0,0}};
//static const SLEDPROGRAM prog_c = {C_SCEN_C_COUNT, (SLEDSCENE*)scens_c};


TLEDECECT::TLEDECECT ()
{
	NextProgramTimer = C_NEXT_STARTLED_TIME;
	PeriodControl = 0;
	SYSBIOS::ADD_TIMER_SYS (&PeriodControl);
	SYSBIOS::ADD_TIMER_SYS (&NextProgramTimer);
	c_effect_var_depth = -C_UKRANIAN_EF_SIZE;
	stl_ix = 0;
	effect_var = 0;
	effect_var_step = C_UKRANIAN_EF_STEP;
	effect_point_pos = C_FLY_EF_POINTIX;
	effect_point_pos_step = C_FLY_EF_STEP;
	AddObjectToExecuteManager ();
}


		
void TLEDECECT::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	__HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStructure.Pin = C_EXTLED1_PIN | C_EXTLED2_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 
	GPIO_InitStructure.Pull = GPIO_PULLUP; 
  HAL_GPIO_Init (C_EXTLED1_2_PORT, &GPIO_InitStructure);
	C_EXTLED1_2_PORT->BRR = C_EXTLED1_PIN;
	C_EXTLED1_2_PORT->BRR = C_EXTLED2_PIN;
}



void TLEDECECT::next_program_start ()
{
unsigned char cur_sw = sw;
cur_sw++;
if (cur_sw >= ELEFCT_ENDENUM) cur_sw = ELEFCT_A;
sw = (ELEFCT)cur_sw;
StartProgramm ((ELEFCT)cur_sw);
}



void TLEDECECT::Task ()
{
	if (!PeriodControl)
		{
		if (!NextProgramTimer)
			{
			next_program_start ();
			NextProgramTimer = C_NEXT_STARTLED_TIME;
			return;
			}
			
		if (state.prog_state == ELPROGSTATE_PLAY)
			{
			
			//sw = ELEFCT_NONE;
				
			switch (sw)
				{
				case ELEFCT_A:
				case ELEFCT_C:
					{
					if (state.gradient_state.TimeTransition)
						{
						scene_gradient_update_10ms ();
						state.gradient_state.TimeTransition--;
						}
					else
						{
						if (state.gradient_state.TimeWaiting)
							{
							state.gradient_state.TimeWaiting--;
							}
						else
							{
							scene_next ();
							}
						}
					update_led_phisical ();
					break;
					}
				case ELEFCT_B:
					{
					effect_ukraine ();
					phisical_many_leds_update ();
					break;
					}
				case ELEFCT_D:
					{
					effect_fly (true);
					phisical_many_leds_update ();
					break;
					}
				case ELEFCT_E:
					{
					effect_fly (false);
					phisical_many_leds_update ();
					break;
					}
				default:
					{
					}
				}
			}
		PeriodControl = 10;
		}
}



void TLEDECECT::calculate_scene_gradient (SLEDSCENE *cur_scn, SLEDSCENE *next_scn)
{
if (state.lp_prog && next_scn && cur_scn)
	{
	unsigned long ix = 0;
	float val1,val2, delt;
	while (ix < C_LEDS_AMOUNT)
		{
		// gradient step calculate color 'R';
		val2 = next_scn->rgb[ix].r;
		val1 = cur_scn->rgb[ix].r;
		delt = val2 - val1;
		if (cur_scn->TimeTransition)
			{
			delt /= cur_scn->TimeTransition;
			state.gradient_state.step[ix].r = delt;
			}
		else
			{
			state.gradient_state.step[ix].r = 0;
			}
		state.gradient_state.rgb[ix].r = val1;
		// gradient step calculate color 'G';	
		val2 = next_scn->rgb[ix].g;
		val1 = cur_scn->rgb[ix].g;
		delt = val2 - val1;	
		if (cur_scn->TimeTransition)	
			{
			delt /= cur_scn->TimeTransition;
			state.gradient_state.step[ix].g = delt;	
			}
		else
			{	
			state.gradient_state.step[ix].g = 0;
			}
		state.gradient_state.rgb[ix].g = val1;
		// gradient step calculate color 'B';	
		val2 = next_scn->rgb[ix].b;
		val1 = cur_scn->rgb[ix].b;
		delt = val2 - val1;	
		if (cur_scn->TimeTransition)	
			{
			delt /= cur_scn->TimeTransition;
			state.gradient_state.step[ix].b = delt;	
			}
		else
			{	
			state.gradient_state.step[ix].b = 0;
			}
		state.gradient_state.rgb[ix].b = val1;
		ix++;
		}
	}
}




void TLEDECECT::StartProgramm (ELEFCT esw)
{
sw = esw;
switch (esw)
	{
	case ELEFCT_A:
		{
		start_prog (&prog_a);
		break;
		}
	case ELEFCT_B:
		{
		break;
		}
	case ELEFCT_C:
		{
		start_prog (&prog_b);
		break;
		}
	case ELEFCT_D:
	case ELEFCT_E:
		{
		break;
		}
	default:
		{
		stop ();
		break;
		}
	}
}



void TLEDECECT::stop ()
{
if (state.prog_state != ELPROGSTATE_NONE)
	{
	state.prog_state = ELPROGSTATE_NONE;
	unsigned char buf[3] = {0,0,0};
	phisical_one_led_update_a (buf);
	phisical_one_led_update_b (buf);	
	}
}



void TLEDECECT::start_prog (const SLEDPROGRAM *prog)
{
	if (prog)
		{
		state.scene_ix = 0;
		state.lp_prog = (SLEDPROGRAM*)prog;
		scene_set (state.scene_ix);
		state.prog_state = ELPROGSTATE_PLAY;
		}
}



// создает градиент между указаной и следующей сценой
void TLEDECECT::scene_set (unsigned short scnix)
{
if (state.lp_prog)
	{
	unsigned short next_scix = next_scene_ix (scnix);
	// подсчитываем шаг градиента
	calculate_scene_gradient (&state.lp_prog->scene[scnix], &state.lp_prog->scene[next_scix]);
	state.gradient_state.TimeTransition = state.lp_prog->scene[scnix].TimeTransition;
	state.gradient_state.TimeWaiting = state.lp_prog->scene[scnix].TimeWaiting;
	}
}



void TLEDECECT::scene_next ()
{
if (state.lp_prog)
	{
	// устанавливаем текущюю сцену
	scene_set (state.scene_ix);
	state.scene_ix++;
	if (state.scene_ix >= state.lp_prog->scene_count) state.scene_ix = 0;	// новый круг той же программы
	}
}



void TLEDECECT::scene_gradient_update_10ms ()
{
if (state.lp_prog)
	{
	unsigned long ix = 0;
	while (ix < C_LEDS_AMOUNT)
		{
		state.gradient_state.rgb[ix].r += state.gradient_state.step[ix].r;
		state.gradient_state.rgb[ix].g += state.gradient_state.step[ix].g;
		state.gradient_state.rgb[ix].b += state.gradient_state.step[ix].b;
		ix++;
		}
	}
}



unsigned short TLEDECECT::next_scene_ix (unsigned short scn_ix)
{
	unsigned short rv_ix = 0;
	if (state.lp_prog)
		{
		unsigned short nix = scn_ix + 1;
		if (nix < state.lp_prog->scene_count) {
			rv_ix = scn_ix + 1;
			}
		}
	return rv_ix;
}


void TLEDECECT::bit_tx_a (bool val)
{
		C_EXTLED1_2_PORT->BSRR = C_EXTLED1_PIN;
		S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			C_EXTLED1_2_PORT->BRR = C_EXTLED1_PIN;
			}
		else
			{
			C_EXTLED1_2_PORT->BRR = C_EXTLED1_PIN;
			S_NOP();
			}
}



void TLEDECECT::bit_tx_b (bool val)
{
		C_EXTLED1_2_PORT->BSRR = C_EXTLED2_PIN;
		S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			C_EXTLED1_2_PORT->BRR = C_EXTLED2_PIN;
			}
		else
			{
			C_EXTLED1_2_PORT->BRR = C_EXTLED2_PIN;
			S_NOP();
			}
}



void TLEDECECT::byte_color_tx_a (unsigned char colr)
{
unsigned char cntr = 0;
__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();
while (cntr < 8)
	{
	bit_tx_a (colr & 0x80);
	colr <<=1;
	cntr++;	
	}
__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}



void TLEDECECT::byte_color_tx_b (unsigned char colr)
{
unsigned char cntr = 0;
__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();
while (cntr < 8)
	{
	bit_tx_b (colr & 0x80);
	colr <<=1;
	cntr++;	
	}
__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}



void TLEDECECT::phisical_full_line_a (SLEDSDF *rgb)
{
	if (rgb)
		{
		unsigned long ix = 0;
		unsigned char buf[3];
		buf[0] = rgb->r / C_LIGHT_DIVIDER; 
		buf[1] = rgb->g / C_LIGHT_DIVIDER; 
		buf[2] = rgb->b / C_LIGHT_DIVIDER; 
		while (ix < C_LEDS_PCBLINE)
			{
			phisical_one_led_update_a (buf);
			ix++;
			}
		}
}



void TLEDECECT::phisical_full_line_b (SLEDSDF *rgb)
{
	if (rgb)
		{
		unsigned long ix = 0;
		unsigned char buf[3];
		buf[0] = rgb->r / C_LIGHT_DIVIDER; 
		buf[1] = rgb->g / C_LIGHT_DIVIDER; 
		buf[2] = rgb->b / C_LIGHT_DIVIDER; 
		while (ix < C_LEDS_PCBLINE)
			{
			phisical_one_led_update_b (buf);
			ix++;
			}
		}
}



void TLEDECECT::update_led_phisical ()
{
phisical_full_line_a (&state.gradient_state.rgb[0]);
phisical_full_line_b (&state.gradient_state.rgb[1]);	
}



void TLEDECECT::phisical_one_led_update_a (unsigned char buf[3])
{
		disable_irq ();
		byte_color_tx_a (buf[1]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		byte_color_tx_a (buf[0]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		byte_color_tx_a (buf[2]);
		enable_irq ();	
}



void TLEDECECT::phisical_one_led_update_b (unsigned char buf[3])
{
		disable_irq ();
		byte_color_tx_b (buf[1]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		byte_color_tx_b (buf[0]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		byte_color_tx_b (buf[2]);
		enable_irq ();	
}




void TLEDECECT::phisical_many_led_update_a (SLEDSD leds[C_LEDS_PCBLINE])
{
	unsigned short ix = 0;
	while (ix < C_LEDS_PCBLINE)
		{
		phisical_one_led_update_a ((unsigned char*)&leds[ix]);
		ix++;
		}
}




void TLEDECECT::phisical_many_led_update_b (SLEDSD leds[C_LEDS_PCBLINE])
{
	unsigned short ix = 0;
	while (ix < C_LEDS_PCBLINE)
		{
		phisical_one_led_update_b ((unsigned char*)&leds[ix]);
		ix++;
		}
}



void TLEDECECT::phisical_many_leds_update ()
{
	phisical_many_led_update_a (leds_arr_a);
	phisical_many_led_update_b (leds_arr_b);
}



void TLEDECECT::clear_array_a ()
{
	memset (leds_arr_a, 0, sizeof(leds_arr_a));
}



void TLEDECECT::clear_array_b ()
{
	memset (leds_arr_b, 0, sizeof(leds_arr_b));
}
		


void TLEDECECT::clear_arrays ()
{
	clear_array_a ();
	clear_array_b ();
}



void TLEDECECT::point_set (SLEDSD *dst, unsigned char point_ix, unsigned char size, ELCOLOR fone_c)
{
	if (dst)
		{
		if (size > (C_LEDS_PCBLINE/2)) size = C_LEDS_PCBLINE/2;
		if (point_ix < C_LEDS_PCBLINE && size)
			{
			SLEDSD *src_color = (SLEDSD*) &color_code[fone_c];
	
			unsigned char back_sz = size / 2;
			unsigned char forw_sz = (size / 2) + (size % 2);
			unsigned char start_ix = point_ix;
			if (start_ix >= back_sz) 
				{
				start_ix = start_ix - back_sz;
				}
			else
				{
				start_ix = 0;
				}
			unsigned char stop_ix = point_ix;
			if ((stop_ix + forw_sz) < C_LEDS_PCBLINE) 
				{
				stop_ix += forw_sz;
				}
			else
				{
				stop_ix = C_LEDS_PCBLINE - 1;
				}
			// отрисовка пиксела
			while (start_ix <= stop_ix)
				{
				dst[start_ix] = *src_color;
				start_ix++;
				}
			}
		}
}



void TLEDECECT::line_fill (SLEDSD *dst, unsigned char p_ix_start, unsigned char p_ix_stop, ELCOLOR fone_c)
{
if (dst)
	{
	unsigned char wr_ix_start, wr_ix_stop, len;
	if (p_ix_start > p_ix_stop)
		{
		wr_ix_start = p_ix_stop;
		wr_ix_stop = p_ix_start;
		}
	else
		{
		wr_ix_start = p_ix_start;
		wr_ix_stop = p_ix_stop;
		}
	if (wr_ix_start >= C_LEDS_PCBLINE) return;
	if (wr_ix_stop >= C_LEDS_PCBLINE) wr_ix_stop = C_LEDS_PCBLINE - 1;
	len = wr_ix_stop - wr_ix_start + 1; 
	if (len)
		{
		SLEDSD *src_color = (SLEDSD*) &color_code[fone_c];
		while (len)
			{
			dst[wr_ix_start++] = *src_color;
			len--;
			}
		}
	}
}



void TLEDECECT::color_mux (SLEDSD *in1, SLEDSD *in2, SLEDSD *out)
{
	if (in1 && in2 && out)
		{
		// R
		unsigned short clr = in1->r / 2;
		clr += in2->r / 2;
		if (clr > 255) clr = 255;
		out->r = clr;
		// G
		clr = in1->g / 2;
		clr += in2->g / 2;
		if (clr > 255) clr = 255;
		out->g = clr;
		// B
		clr = in1->b / 2;
		clr += in2->b / 2;
		if (clr > 255) clr = 255;
		out->b = clr;
		}
}



void TLEDECECT::line_blur (SLEDSD *dst, unsigned char p_ix_start, unsigned char p_ix_stop)
{
if (dst)	
	{
	unsigned char wr_ix_start, wr_ix_stop, len;
	if (p_ix_start > p_ix_stop)
		{
		wr_ix_start = p_ix_stop;
		wr_ix_stop = p_ix_start;
		}
	else
		{
		wr_ix_start = p_ix_start;
		wr_ix_stop = p_ix_stop;
		}
	if (wr_ix_start >= C_LEDS_PCBLINE) return;
	if (wr_ix_stop >= C_LEDS_PCBLINE) wr_ix_stop = C_LEDS_PCBLINE - 1;
	len = wr_ix_stop - wr_ix_start; 
	if (len)
		{
		SLEDSD rgb;	
		while (len)
			{
			color_mux (&dst[wr_ix_start], &dst[wr_ix_start+1], &rgb);
			dst[wr_ix_start] = rgb;
			//dst[wr_ix_start + 1] = rgb;
			wr_ix_start++;
			len--;
			}
		}
	}
}




		//float effect_var_step;
/*
void TLEDECECT::effect_ukraine ()
{
	
	unsigned char p_ix_start_blue = 0;
	long p_ix_stop_blue = C_LEDS_PCBLINE / 2;

		
	effect_var += effect_var_step;
	if (effect_var > C_UKRANIAN_EF_SIZE || effect_var < -C_UKRANIAN_EF_SIZE) 
		{
		effect_var_step = effect_var_step * -1;
		}
		
	p_ix_stop_blue += effect_var;
	
	unsigned char p_ix_start_yellow = p_ix_stop_blue;
	unsigned char p_ix_stop_yellow = C_LEDS_PCBLINE - 1;
	
	line_fill (leds_arr_a, p_ix_start_blue, p_ix_stop_blue, ELCOLOR_BLUE);
	line_fill (leds_arr_a, p_ix_start_yellow, p_ix_stop_yellow, ELCOLOR_YELLOW);
	line_fill (leds_arr_b, p_ix_start_blue, p_ix_stop_blue, ELCOLOR_BLUE);
	line_fill (leds_arr_b, p_ix_start_yellow, p_ix_stop_yellow, ELCOLOR_YELLOW);
}
*/


void TLEDECECT::effect_ukraine ()
{
	
	unsigned char p_ix_start_blue = 0;
	long p_ix_stop_blue = C_LEDS_PCBLINE / 2;
	long blur_start_pos = p_ix_stop_blue - effect_var - 5;
	long blur_stop_pos = p_ix_stop_blue + effect_var + 5;

		
	effect_var += effect_var_step;
	if (effect_var > c_effect_var_depth || effect_var < -c_effect_var_depth) 
		{
		static bool f_dir = 1;
		if (stl_ix >= urkshatal.cnt) stl_ix = 0;
		c_effect_var_depth = urkshatal.array[stl_ix].val;
			
		float cstep = urkshatal.array[stl_ix].step;
		if (f_dir) 
			{
			effect_var = c_effect_var_depth;
			cstep = cstep * -1;
			}
		else
			{
			effect_var = -c_effect_var_depth;
			}
		effect_var_step = cstep;
		if (f_dir) stl_ix++;
		f_dir = !f_dir;
		}
		
	p_ix_stop_blue += effect_var;
	
	unsigned char p_ix_start_yellow = p_ix_stop_blue;
	unsigned char p_ix_stop_yellow = C_LEDS_PCBLINE - 1;
	
	line_fill (leds_arr_a, p_ix_start_blue, p_ix_stop_blue, ELCOLOR_BLUE);
	line_fill (leds_arr_a, p_ix_start_yellow, p_ix_stop_yellow, ELCOLOR_YELLOW);
	line_fill (leds_arr_b, p_ix_start_blue, p_ix_stop_blue, ELCOLOR_BLUE);
	line_fill (leds_arr_b, p_ix_start_yellow, p_ix_stop_yellow, ELCOLOR_YELLOW);
		
	unsigned char cnt_bl = 4;
	while (cnt_bl)
		{
		line_blur (leds_arr_a, blur_start_pos, blur_stop_pos);
		line_blur (leds_arr_b, blur_start_pos, blur_stop_pos);
		cnt_bl--;
		}
}


		
		//float effect_point_pos;
		//float effect_point_pos_step;
void TLEDECECT::effect_fly (bool dir)
{
	clear_arrays ();		// cls
	if (dir)
		{
		effect_point_pos += effect_point_pos_step;
		}
	else
		{
		effect_point_pos -= effect_point_pos_step;
		}
	if (effect_point_pos >= C_LEDS_PCBLINE) effect_point_pos = 0;
	if (effect_point_pos < 0) effect_point_pos = C_LEDS_PCBLINE - 1;

	point_set (leds_arr_a, effect_point_pos, C_FLY_EF_PIXSIZE, ELCOLOR_WHITE);
	point_set (leds_arr_b, effect_point_pos, C_FLY_EF_PIXSIZE, ELCOLOR_WHITE);
}





