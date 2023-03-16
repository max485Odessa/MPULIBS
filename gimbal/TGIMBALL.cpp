#include "TGIMBALL.h"
#include <math.h>

//extern void SetRollMotor (float , int);
//extern void SetPitchMotor (float , int);
//extern void SetYawMotor (float , int);

uint32_t TGIMBALL::WatchDogCounter = 0;


TGIMBALL::TGIMBALL (TMEMIF *mm)		// TBMOTORRAW *mtr,
{
InitSinArray ();
memory = mm;
imu = TMPU6050::imuobj ();
	
clear_state (&motor_states[ROLL]);
clear_state (&motor_states[PITCH]);
clear_state (&motor_states[YAW]);

motor_states[ROLL].Timer = TIM2;
motor_states[PITCH].Timer = TIM1;
motor_states[YAW].Timer = TIM8;
	
f_gimbal_state = false;
SetDebugStates (false);
clear_debug_info ();
}




float TGIMBALL::constrain (float value, float low, float high)
{
    if (value < low)
        return low;

    if (value > high)
        return high;

    return value;
}



float TGIMBALL::minimal (float value, float low)
{
	float rv = value;
	if (rv < 0)
		{
		if (rv > (-low)) rv = -low;
		}
	else
		{
		if (rv < low) rv = low;
		}
    return rv;
}




void TGIMBALL::On ()
{
	if (f_gimbal_state != true)
		{
		pwm_out_roll_enable (false);
		pwm_out_pitch_enable (false);
		pwm_out_yaw_enable (false);
		
		reset_axis_data (&motor_states[ROLL]);
		reset_axis_data (&motor_states[PITCH]);
		reset_axis_data (&motor_states[YAW]);
			
		pwm_out_roll_enable (true);
		pwm_out_pitch_enable (true);
		pwm_out_yaw_enable (true);
			
		ic_enable (ROLL, true);	
		ic_enable (PITCH, true);
		ic_enable (YAW, true);
			
		InitPWMTimers (PWM_MOTOR_PERIOD, C_MOTOR_TIMERS_SPEED);
		
		f_gimbal_state = true;
		}
}



void TGIMBALL::Off ()
{
	if (f_gimbal_state == true)
		{
		pwm_out_roll_enable (false);
		pwm_out_pitch_enable (false);
		pwm_out_yaw_enable (false);
			
		ic_enable (ROLL, false);	
		ic_enable (PITCH, false);	
		ic_enable (YAW, false);	
		
			
			
		TIM_Cmd (motor_states[ROLL].Timer, DISABLE);
		TIM_Cmd (motor_states[PITCH].Timer, DISABLE);
		TIM_Cmd (motor_states[YAW].Timer, DISABLE);
			
		f_gimbal_state = false;
		}
}




void TGIMBALL::fillmemf (float *dst, uint32_t cnt, float dt)
{
	if (dst)
		{
		while (cnt)
			{
			*dst++ = dt;
			cnt--;
			}
		}
}




void TGIMBALL::clear_state (S_MOTOR_STATE_T *lp)
{
	if (lp) {
		lp->border.f_active = false;
		lp->c_angl_offset = 0;
		lp->last_error = 0;
		lp->c_local_coef_I = 0;
		lp->f_hold_state = false;
		lp->f_reg_p_on = false;
		lp->f_reg_i_on = false;
		lp->f_reg_d_on = false;
		lp->c_program_angl = 0;
		lp->f_inverse_motor = false;
		lp->f_inverse_sens = false;
		}
}



void TGIMBALL::update_move (tAxis ax)
{
	if (ax >= NUMAXIS) return;
	S_MOTOR_STATE_T &motr = motor_states[ax];
	if (motr.border.f_active) {
		if (motr.c_program_angl > motr.border.angle_max) motr.c_program_angl = motr.border.angle_max;
		if (motr.c_program_angl < motr.border.angle_min) motr.c_program_angl = motr.border.angle_min;
		}
	corrected360 (motr.c_program_angl);
}







// функция добавляющая смещение к указаному углу
float TGIMBALL::angle_add_circle (float val, float offs)
{
float rv = val + offs;
if (rv >= C_MAXANGLE_VALUE) rv = rv - C_MAXANGLE_VALUE;
if (rv < 0) rv = C_MAXANGLE_VALUE - rv;
return rv;
}



float TGIMBALL::angle_add_border (float val, float offs, float angl_min, float angl_max)
{
	float rv = val + offs;
	if (rv >= angl_max) rv = angl_max;
	if (rv < angl_min) rv = angl_min;
	return rv;
}

float TGIMBALL::inverse_angle (float anggl)
{
return 360.0F - anggl;
}




uint32_t TGIMBALL::steps_for_one_angle (const S_MOTOR_STATE_T &state)
{
uint32_t speps_for_360 = PWM_MOTOR_PERIOD * state.poles;
float stp = speps_for_360;  stp /= 360.0F;
return stp;
//#define C_360_STEPS (PWM_MOTOR_PERIOD * C_MOTOR_PHASES)			// количество шагов на 360 градусов
//#define C_STEPS_FOR_ANGLE (C_360_STEPS / 360.0F)	
}







void TGIMBALL::set_motor_phase (S_MOTOR_STATE_T &state, float angl_motor)
{
//state.pwm_state.phi_step = 0;
state.pwm_state.phi = get_phase_index (state, angl_motor);
//state.pwm_state.phi_counts = 1;
}



/*
// расчитывает время перемещения мотора на угловой отрезок (угловую дельту) со средней скоростью
float TGIMBALL::get_midle_time (uint16_t start_rpm, uint16_t stop_rpm, float angl_delt)
{
	float rv = 0, midl_rpm = (stop_rpm + start_rpm);
	midl_rpm /= 2;

	return rv;
}
*/


// входящее угловое положение мотора уже должно быть скорректировано с положением магнитов ротора
uint16_t TGIMBALL::get_phase_index (const S_MOTOR_STATE_T &state, float angl_motor)
{
const float phase_step = 360.0F / state.poles;			// градусов на полюс
//angl_motor += dst->c_angl_offset;
long valc = (angl_motor / phase_step);
float valo = angl_motor - (phase_step * valc);		// дробные части 
float quant_angl = SINARRAYSIZE; quant_angl /= phase_step;
quant_angl *= ( phase_step - valo); // phase_step - valo phase_step -
return quant_angl;
}


/*
// входящее угловое положение мотора уже должно быть ранее скорректировано с положением магнитов ротора
// по углу определяется фазовое положение ротора. По полярности angl_dlt и текущему фазовому углу индекс синус таблицы.
void TGIMBALL::aply_move (S_MOTOR_STATE_T &state, float mot_pos, float angl_dlt)
{
float angl_to_poles = (360.0F / state.poles) ;       // magnet poles angle size
float angl_to_poles_q = angl_to_poles / 6;           // magnet poles angle size
float val = angl_dlt * state.c_coef_P;		// p
if (val < 0) val *= -1.0;
if (val > angl_to_poles_q) val = angl_to_poles_q;
if (angl_dlt < 0)
	{
	mot_pos -= val;
	}
else
	{
	mot_pos += val;
	}

float ix_to_angl = SINARRAYSIZE; ix_to_angl /= angl_to_poles;
float ph_cnt = mot_pos / angl_to_poles;                        			// current magnet pole count
float start_phase_pos = (angl_to_poles * (long)ph_cnt);            // encoder sensor position == start magnetic phases
float mot_phase_offs = mot_pos - start_phase_pos;

ix_to_angl *= (angl_to_poles - mot_phase_offs); // phase_step - valo phase_step -
//if (angl_dlt < 0) ix_to_angl = (SINARRAYSIZE) - ix_to_angl;
uint16_t ix = ix_to_angl;// (12.0F * ix_to_angl);  //  SINARRAYSIZE - SINARRAYSIZE
state.pwm_state.phi = ix;
	

}
*/





float TGIMBALL::get_circular_angle_delta (float curpos, float needpos)
{
	float delt = 0;
	if (curpos > needpos)
		{
		delt = -(curpos - needpos);
		}
	else
		{
		if (curpos < needpos)
			{
			delt = (needpos - curpos);
			}
		}
	return delt;
}


/*
void TGIMBALL::SetSensorsAngles (tAxis ax, float angl_motor, float angl_sens)
{

	//float dltangl = 0;
	//bool f_dir = false;

	S_MOTOR_STATE_T &mst = motor_states[ax];
	if (mst.enable)
		{
		if (mst.f_inverse_motor) angl_motor = inverse_angle (angl_motor);
		// логический угол это угловая позиция без учета физического смещения крепления камеры (угол сенсора положения численно соответствует углу сенсора мотора)
		//ang_s = angle_add_border (ang_s, mst.c_program_angl, mst.border.angle_min, mst.border.angle_max);		// прибавляем логический программный угол сенсора ориентации к программно-установочному логическому углу поворота
		//ang_s = angle_add_circle (ang_s, mst.c_angl_offset);		// прибавляем к углу сенсора ориентации калибровочное физическое смещение для вычисления угла положения сенсора мотора
			
		mst.cur_enc_angle = angl_motor;
		mst.cur_imu_angle = angl_sens;
		float coef_pid = 0;
		float cur_error = mst.c_program_angl - angl_motor;
		float cur_i = 0;
		if (mst.f_reg_i_on)		// включена I регулировка
			{
			// поведенческая модель I регулировки
			// включение,отключение механизма
			if (!mst.local_timer_i_block)		//  таймер блокировки I - не активен
				{
				bool f_c_e_i_dir = false;
				if (cur_error >= 0) f_c_e_i_dir = true;		// полярность текущей ошибки
					
				if (mst.f_prev_i_dir_error != f_c_e_i_dir)
					{
					// полярность ошибки поменялась
					// сбрасываем таймера ошибок и накопительный коефициент I
					mst.local_timer_i_step = mst.c_coef_i_time_step;
					mst.local_timer_i_off = mst.c_coef_i_time_off;
					//mst.c_local_coef_I  = 0;
					mst.f_prev_i_dir_error = f_c_e_i_dir;
					}
					
					
					{
					// полярность ошибки не меняется
					if (!mst.local_timer_i_step)
						{
						// полярность ошибки не менялась с истечении периода инкремента I
						if (!mst.local_timer_i_off)
							{
							// полярность ошибки не менялась долго, заблокировать I регулировку на время
							mst.local_timer_i_block = mst.c_coef_i_relax_time;// 60000;
							mst.local_timer_i_off = mst.c_coef_i_time_off;
							mst.c_local_coef_I = 0;
							}
						else
							{
							// увеличиваем множитель I каждый local_timer_i_step период
							//mst.c_local_coef_I += mst.c_coef_I;
							
							if (f_c_e_i_dir)
								{
								mst.c_local_coef_I += mst.c_coef_I;
								}
							else
								{
								mst.c_local_coef_I += -mst.c_coef_I;
								}
							
							}
						mst.local_timer_i_step = mst.c_coef_i_time_step;
						}
					}
				cur_i = mst.c_local_coef_I;
				}
			else
				{
				// активен таймер блокировки I
				cur_i = 0;
				}
			}
		if (mst.f_reg_p_on) 
			{
			coef_pid += (cur_error * mst.c_coef_P);
			}
		if (mst.f_reg_d_on)
			{
			float coef_d = mst.coef_D * (mst.last_error - cur_error);
			coef_pid -= coef_d;
			}
		
		mst.last_error = cur_error;
		coef_pid += cur_i;
		
		coef_pid = constrain (coef_pid, -mst.c_critical_step_pd, mst.c_critical_step_pd);		// can't be more than 1/8 sinus periode in best variant or maximum 1 radian (57.3) in hard mode
		//coef_pid = minimal (coef_pid, 0.3);

		angl_motor += mst.c_angl_offset;		// совмещение енкодера мотора с фазами магнитов (A = 0, B = 0.5, C = 0.5)
		angl_motor += coef_pid;							
		if (angl_motor < 0.0F) angl_motor += 360.0F;
		if (angl_motor >= 360.0F) angl_motor -= 360.0F;
		float ix_to_angl = SINARRAYSIZE; ix_to_angl /= mst.angles_to_pole;
		long ph_cnt = angl_motor / mst.angles_to_pole;                        			// current magnet pole count
		float start_phase_pos = (mst.angles_to_pole * ph_cnt);            // encoder sensor position == start magnetic phases
		float mot_phase_offs = angl_motor - start_phase_pos;
		ix_to_angl *= mot_phase_offs;		// (mst.angles_to_pole - mot_phase_offs);	
		uint16_t ix = ix_to_angl;
		mst.pwm_state.phi = ix;
			
		
		SetPWMFastTable (mst.pwm_hold_phase_a, mst.pwm_state.phi + 10, mst.power_div);
		SetPWMFastTable (mst.pwm_hold_phase_b, mst.pwm_state.phi - 10, mst.power_div);
		mst.f_hold_state = true;
		SetPWMFastTable (mst.pwm_state.rawpwmval, mst.pwm_state.phi, mst.power_div);
		}
	else
		{
		mst.pwm_state.rawpwmval[0] = PWM_BLANK_VAL;
		mst.pwm_state.rawpwmval[1] = PWM_BLANK_VAL;
		mst.pwm_state.rawpwmval[2] = PWM_BLANK_VAL;
		}
		
	// enable isr
	mst.Timer->SR &= ~TIM_SR_UIF;   // clear UIF flag
	mst.Timer->DIER = TIM_DIER_UIE; // Enable update interrupt
		
	if (f_is_debug_states) debug_info_gen ();
}

*/




void TGIMBALL::SetSensorsAngles (tAxis ax, float angl_motor, float angl_sens)
{

	//float dltangl = 0;
	//bool f_dir = false;

	S_MOTOR_STATE_T &mst = motor_states[ax];
	if (mst.enable)
		{
		if (mst.f_inverse_motor) angl_motor = inverse_angle (angl_motor);
		// логический угол это угловая позиция без учета физического смещения крепления камеры (угол сенсора положения численно соответствует углу сенсора мотора)
		//ang_s = angle_add_border (ang_s, mst.c_program_angl, mst.border.angle_min, mst.border.angle_max);		// прибавляем логический программный угол сенсора ориентации к программно-установочному логическому углу поворота
		//ang_s = angle_add_circle (ang_s, mst.c_angl_offset);		// прибавляем к углу сенсора ориентации калибровочное физическое смещение для вычисления угла положения сенсора мотора
			
		mst.cur_enc_angle = angl_motor;
		mst.cur_imu_angle = angl_sens;
		float coef_pid = 0;
		float cur_error = mst.c_program_angl - angl_motor;
		float cur_i = 0;
		if (mst.f_reg_i_on)		// включена I регулировка
			{
			// поведенческая модель I регулировки
			// включение,отключение механизма
			if (!mst.local_timer_i_block)		//  таймер блокировки I - не активен
				{
				bool f_c_e_i_dir = false;
				if (cur_error >= 0) f_c_e_i_dir = true;		// полярность текущей ошибки
					
				if (mst.f_prev_i_dir_error != f_c_e_i_dir)
					{
					// полярность ошибки поменялась
					// сбрасываем таймера ошибок и накопительный коефициент I
					mst.local_timer_i_step = mst.c_coef_i_time_step;
					mst.local_timer_i_off = mst.c_coef_i_time_off;
					if (f_c_e_i_dir)
						{
						mst.c_local_coef_I += mst.c_coef_I;
						}
					else
						{
						mst.c_local_coef_I += -mst.c_coef_I;
						}
					mst.f_prev_i_dir_error = f_c_e_i_dir;
					}
					
					
					{
					// полярность ошибки не меняется
					if (!mst.local_timer_i_step)
						{
						// полярность ошибки не менялась с истечении периода инкремента I
						if (!mst.local_timer_i_off)
							{
							// полярность ошибки не менялась долго, заблокировать I регулировку на время
							mst.local_timer_i_block = mst.c_coef_i_relax_time;// 60000;
							mst.local_timer_i_off = mst.c_coef_i_time_off;
							mst.c_local_coef_I = 0;
							}
						else
							{
							// увеличиваем множитель I каждый local_timer_i_step период
							//mst.c_local_coef_I += mst.c_coef_I;
							/*
							if (f_c_e_i_dir)
								{
								mst.c_local_coef_I += mst.c_coef_I;
								}
							else
								{
								mst.c_local_coef_I += -mst.c_coef_I;
								}
							*/
							}
						mst.local_timer_i_step = mst.c_coef_i_time_step;
						}
					}
				cur_i = mst.c_local_coef_I;
				}
			else
				{
				// активен таймер блокировки I
				cur_i = 0;
				}
			}
		if (mst.f_reg_p_on) 
			{
			coef_pid += (cur_error * mst.c_coef_P);
			}
		if (mst.f_reg_d_on)
			{
			float coef_d = mst.coef_D * (mst.last_error - cur_error);
			coef_pid -= coef_d;
			}
		
		mst.last_error = cur_error;
		coef_pid += cur_i;
		
		coef_pid = constrain (coef_pid, -mst.c_critical_step_pd, mst.c_critical_step_pd);		// can't be more than 1/8 sinus periode in best variant or maximum 1 radian (57.3) in hard mode
		//coef_pid = minimal (coef_pid, 0.3);

		angl_motor += mst.c_angl_offset;		// совмещение енкодера мотора с фазами магнитов (A = 0, B = 0.5, C = 0.5)
		angl_motor += coef_pid;							
		if (angl_motor < 0.0F) angl_motor += 360.0F;
		if (angl_motor >= 360.0F) angl_motor -= 360.0F;
		float ix_to_angl = SINARRAYSIZE; ix_to_angl /= mst.angles_to_pole;
		long ph_cnt = angl_motor / mst.angles_to_pole;                        			// current magnet pole count
		float start_phase_pos = (mst.angles_to_pole * ph_cnt);            // encoder sensor position == start magnetic phases
		float mot_phase_offs = angl_motor - start_phase_pos;
		ix_to_angl *= mot_phase_offs;		// (mst.angles_to_pole - mot_phase_offs);	
		uint16_t ix = ix_to_angl;
		mst.pwm_state.phi = ix;
			
		
		//SetPWMFastTable (mst.pwm_hold_phase_a, mst.pwm_state.phi + 6, mst.power_div);
		//SetPWMFastTable (mst.pwm_hold_phase_b, mst.pwm_state.phi - 6, mst.power_div);
		//mst.f_hold_state = true;
		SetPWMFastTable (mst.pwm_state.rawpwmval, mst.pwm_state.phi, mst.power_div);
		}
	else
		{
		mst.pwm_state.rawpwmval[0] = PWM_BLANK_VAL;
		mst.pwm_state.rawpwmval[1] = PWM_BLANK_VAL;
		mst.pwm_state.rawpwmval[2] = PWM_BLANK_VAL;
		}
		
	// enable isr
	mst.Timer->SR &= ~TIM_SR_UIF;   // clear UIF flag
	mst.Timer->DIER = TIM_DIER_UIE; // Enable update interrupt
		
	if (f_is_debug_states) debug_info_gen ();
}









void TGIMBALL::debug_info_gen ()
{
	char ix = 0;
	while (ix < ENDENUM_AXIS)
		{
		S_MOTOR_STATE_T &mst = motor_states[ix];			
		debug_states.enc[ix] = mst.cur_enc_angle;
		debug_states.imu[ix] = mst.cur_imu_angle;
		debug_states.set[ix] = mst.c_program_angl;
		debug_states.error[ix] = mst.last_error;			
		debug_states.value_i[ix] = 0;
		debug_states.active_i[ix] = 0;
		if (mst.f_reg_i_on) {
				if (mst.local_timer_i_off) {
					debug_states.active_i[ix] = true;
					debug_states.value_i[ix] =  mst.c_local_coef_I;
				}
			}
		ix++;
		}
	f_debug_info_complete_ok = true;
}



void TGIMBALL::clear_debug_info ()
{
	char ix = 0;
	while (ix < ENDENUM_AXIS)
		{
		//S_MOTOR_STATE_T &mst = motor_states[ix];			
		debug_states.enc[ix] = 0;
		debug_states.imu[ix] = 0;
		debug_states.error[ix] = 0;			
		debug_states.value_i[ix] = 0;
		debug_states.active_i[ix] = 0;
		ix++;
		}
	f_debug_info_complete_ok = false;
}



S_GSTATEBODY_T *TGIMBALL::GetDebugInfo ()
{
S_GSTATEBODY_T *rv = 0;
if (f_debug_info_complete_ok) rv = &debug_states;
return rv;	
}



void TGIMBALL::SetDebugStates (bool val)
{
	f_is_debug_states = val;
}


#define C_PERIOD_HOLD 1




int *TGIMBALL::tim_isr_period_roll ()
{
int *rv = motor_states[ROLL].pwm_state.rawpwmval;
	if (motor_states[ROLL].f_hold_state)
		{
		bool c_phase = motor_states[ROLL].f_hold_phase;
		if (c_phase)
			{
			rv = motor_states[ROLL].pwm_hold_phase_a;
			}
		else
			{
			rv = motor_states[ROLL].pwm_hold_phase_b;
			}
			
		if (motor_states[ROLL].period_hold) motor_states[ROLL].period_hold--;
		if (!motor_states[ROLL].period_hold)	
			{
			motor_states[ROLL].period_hold = C_PERIOD_HOLD;
			motor_states[ROLL].f_hold_phase = !c_phase;
			}

		}
	return rv;
}



int *TGIMBALL::tim_isr_perios_pitch ()
{
int *rv = motor_states[PITCH].pwm_state.rawpwmval;
	if (motor_states[PITCH].f_hold_state)
		{
		bool c_phase = motor_states[PITCH].f_hold_phase;
		if (c_phase)
			{
			rv = motor_states[PITCH].pwm_hold_phase_a;
			}
		else
			{
			rv = motor_states[PITCH].pwm_hold_phase_b;
			}
		if (motor_states[PITCH].period_hold) motor_states[PITCH].period_hold--;
		if (!motor_states[PITCH].period_hold)	
			{
			motor_states[PITCH].period_hold = C_PERIOD_HOLD;
			motor_states[PITCH].f_hold_phase = !c_phase;
			}
		}
	return rv;
}



int *TGIMBALL::tim_isr_period_yaw ()
{
int *rv = motor_states[YAW].pwm_state.rawpwmval;
	if (motor_states[YAW].f_hold_state)
		{
		bool c_phase = motor_states[YAW].f_hold_phase;
		if (c_phase)
			{
			rv = motor_states[YAW].pwm_hold_phase_a;
			}
		else
			{
			rv = motor_states[YAW].pwm_hold_phase_b;
			}
		if (motor_states[YAW].period_hold) motor_states[YAW].period_hold--;
		if (!motor_states[YAW].period_hold)	
			{
			motor_states[YAW].period_hold = C_PERIOD_HOLD;
			motor_states[YAW].f_hold_phase = !c_phase;
			}
		}
	return rv;
}



void TGIMBALL::SetAngle_prog (tAxis ax, float angl)
{
	if (ax < NUMAXIS)  {
		motor_states[ax].c_program_angl = angl;
		}
}



// input power value: 1-255
void TGIMBALL::SetMotorPower (tAxis ax, uint16_t pwrval)
{
	if (ax < NUMAXIS) 
		{
		if (pwrval > C_MAXPOWER_VALUE) pwrval = C_MAXPOWER_VALUE;
		motor_states[ax].power_div = C_MAXPOWER_VALUE - pwrval + 1;		// out power divider: 256-1
		}
}




void TGIMBALL::SetBordersAngl (tAxis ax, float angl_min, float angl_max)
{
	if (ax < NUMAXIS) {
		motor_states[ax].border.angle_max = angl_max;
		motor_states[ax].border.angle_min = angl_min;
		}
}



void TGIMBALL::SetTimeStep_I (tAxis ax, uint32_t val)
{
	if (ax < NUMAXIS)  {	
		motor_states[ax].c_coef_i_time_step = val;
		//motor_states[ax].f_i_stoped = false;
		}
}



void TGIMBALL::SetTimeOff_I (tAxis ax, uint32_t val)
{
	if (ax < NUMAXIS)  {
		motor_states[ax].c_coef_i_time_off = val;
		//motor_states[ax].f_i_stoped = false;
		}
}



void TGIMBALL::SetTimeRelax_I (tAxis ax, uint32_t vall)
{
	if (ax < NUMAXIS) motor_states[ax].c_coef_i_relax_time = vall;
}



void TGIMBALL::SetEnable_Border (tAxis ax, bool f_val)
{
	if (ax < NUMAXIS) motor_states[ax].border.f_active = f_val;
}



void TGIMBALL::SetValue_P (tAxis ax, float val)
{
	if (ax < NUMAXIS) motor_states[ax].c_coef_P = val;
}



void TGIMBALL::SetValue_I (tAxis ax, float val)
{
	if (ax < NUMAXIS)  {
		motor_states[ax].c_coef_I = val;
		motor_states[ax].c_local_coef_I = 0;//val;
		//motor_states[ax].f_i_stoped = false;
		}
}



void TGIMBALL::SetValue_D (tAxis ax, float val)
{
	if (ax < NUMAXIS) motor_states[ax].coef_D = val;
}



void TGIMBALL::SetEnable_P (tAxis ax, bool f_val)
{
	if (ax < NUMAXIS) motor_states[ax].f_reg_p_on = f_val;
}



void TGIMBALL::SetEnable_I (tAxis ax, bool f_val)
{
	if (ax < NUMAXIS) 
		{
		motor_states[ax].f_reg_i_on = f_val;
		//motor_states[ax].f_i_stoped = false;
		}
}



void TGIMBALL::SetEnableAxis (tAxis ax, bool f_val)
{
	if (ax < NUMAXIS) 
		{
		if (f_val)
			{
			motor_states[ax].local_timer_i_step = motor_states[ax].c_coef_i_time_step;
			motor_states[ax].local_timer_i_off = motor_states[ax].c_coef_i_time_off;
			motor_states[ax].local_timer_i_block = 0;
			motor_states[ax].c_local_coef_I = 0;
			motor_states[ax].f_prev_i_dir_error = false;
			}
		motor_states[ax].enable = f_val;
		}
}



void TGIMBALL::SetEnable_D (tAxis ax, bool f_val)
{
	if (ax < NUMAXIS) motor_states[ax].f_reg_d_on = f_val;
}



void TGIMBALL::SetAnglOffset (tAxis ax, float angl)
{
	if (ax < NUMAXIS) motor_states[ax].c_angl_offset = angl;
}



void TGIMBALL::SetMotorPoles (tAxis ax, uint16_t poles)
{
	if (!poles) poles = 1;
	if (ax < NUMAXIS) 
		{
		motor_states[ax].poles = poles;
		motor_states[ax].angles_to_pole = 360.0F / poles;
		motor_states[ax].c_critical_step_pd = motor_states[ax].angles_to_pole / 8;
		}
}



void TGIMBALL::SetInverseAnglMotor (tAxis ax, bool inv)
{
	if (ax < NUMAXIS) motor_states[ax].f_inverse_motor = inv;
}



void TGIMBALL::SetInverseAnglSens (tAxis ax, bool inv)
{
	if (ax < NUMAXIS) motor_states[ax].f_inverse_sens = inv;
}



/*
void TGIMBALL::Task ()
{
	static volatile uint8_t idlsm;
	//idlsm = TLSM6DSL::objsif ()->get_id ();
	//idlsm++;
		//TLSM6DSL::objsif ()->get_acc_sync (AccData);
		//TLSM6DSL::objsif ()->get_gyro_sync (GyroData);
}
*/






/*
// перемещение по времени
float TGIMBALL::ads_step_time (uint16_t rpm_1, uint16_t rpm_2, float timesk, uint32_t *dstcnt)
{
	float step_phi_start = step_for_rpm (rpm_1);
	float step_phi_stop = step_for_rpm (rpm_2);
	uint32_t pwm_counts = timesk / C_PWMPERIOD_F;		// количество PWM циклов за указанное время
	float rv = (step_phi_stop - step_phi_start) / pwm_counts;
	if (dstcnt) *dstcnt = pwm_counts;
	return rv;
}
*/





