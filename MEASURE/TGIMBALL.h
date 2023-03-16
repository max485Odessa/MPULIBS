#ifndef _H_GIMBALL_MOTORS_H_
#define _H_GIMBALL_MOTORS_H_



#include "stm32f30x_tim.h"
#include "stm32f30x_comp.h"
#include "stm32f30x_dac.h"
#include "stm32f30x_rcc.h"
#include "definitions.h"
#include "hard_rut.h"
#include "TINDUCT.h"
#include "TFTASKIF.h"
#include "SYSBIOS.H"
#include "utils.h"
#include "TGBLCMD.h"
#include "TMPU6050.h"
#include "stopwatch.h"
#include "TMEMIFACE.h"


#define DEADBAND 2.0f //in radians with respect to one motor pole (actual angle is (DEADBAND / numberPoles) * R2D)
#define MOTORPOS2SETPNT 0.55f //scaling factor for how fast it should move
#define AUTOPANSMOOTH 40.0f
#define PITCH_UP_LIMIT (-50 * D2R)
#define PITCH_DOWN_LIMIT (90 * D2R)
#define CORRECTION_STEP 1.0F




enum EMPROGIX {EMPROGIX_ACC = 0, EMPROGIX_DRIVE, EMPROGIX_BRAKING, EMPROGIX_HOLD, EMPROGIX_ENDENUM};



typedef struct {
	float x;
	float y;
	float z;
} S_ANGLORIENT_T;


typedef struct {
	S_MOTPROGRM_T *prg_x[EMPROGIX_ENDENUM];
	S_MOTPROGRM_T *prg_y[EMPROGIX_ENDENUM];
	S_MOTPROGRM_T *prg_z[EMPROGIX_ENDENUM];
} S_LPPRG3AXIS_T;		// указатели на 4x-шаговые программы перемещени€ по каждым ос€м







#define C_MINNOINERTION_RPM 20


class TGIMBALL: public TINDIF {				// , public TFFC
		//virtual void Task ();
		virtual int *tim_isr_period_roll ();
		virtual int *tim_isr_perios_pitch ();
		virtual int *tim_isr_period_yaw ();
	
		void fillmemf (float *dst, uint32_t cnt, float dt);
	
		//S_MOTOR_GBL_T motor_states[NUMAXIS];
		
		TSENIMU_IF *imu;
		//TBMOTORRAW *motors;
		TMEMIF *memory;
		bool f_gimbal_state;
		float constrain(float value, float low, float high);
		
			void clear_state (S_MOTOR_STATE_T *lp);

			uint32_t steps_for_one_angle (const S_MOTOR_STATE_T &state);
			float angle_add_circle (float val, float offs);		// функци€ добавл€юща€ смещение к указаному углу
			float angle_add_border (float val, float offs, float angl_min, float angl_max);		// функци€ добавл€юща€ смещение к указаному углу и контролирующа€ ограничени€
			float inverse_angle (float anggl);
			
			//bool update_pwm_state (tAxis ax, float angl, bool dir);
			//uint16_t get_cur_rpm (const S_MOTOR_STATE_T &state);
			//float step_for_rpm (uint16_t rpm);
			//void aply_move (S_MOTOR_STATE_T &state, float mot_pos, float angl_dlt);
			//uint32_t steps_for_delta_angle (uint16_t rpm,  float pos_angl, float dlt_angl, const S_MOTOR_STATE_T &state);		// , uint32_t *dstcnt
			//float get_midle_time (uint16_t start_rpm, uint16_t stop_rpm, float angl_delt);
			//bool next_step_isr (S_MOTOR_STATE_T &state);
			//uint16_t calc_speed (float dltangl, const S_MOTOR_STATE_T &state);
			
			float get_circular_angle_delta (float curpos, float needpos);
			void update_move (tAxis ax);
		
			bool f_settings_update;
			uint16_t get_phase_index (const S_MOTOR_STATE_T &state, float angl_motor);
			void set_motor_phase (S_MOTOR_STATE_T &state, float angl_motor);

			void clear_debug_info ();
			bool f_debug_info_complete_ok;
			S_GSTATEBODY_T debug_states;
			void debug_info_gen ();
			bool f_is_debug_states;
			float minimal (float value, float low);


	public:
			TGIMBALL ( TMEMIF *mm);			// TBMOTORRAW *mtr,
			static uint32_t WatchDogCounter;
			void On ();
			void Off ();
			void SetDebugStates (bool val);
			S_GSTATEBODY_T *GetDebugInfo ();
			void SetSensorsAngles (tAxis ax, float angl_motor, float angl_sens);
	
			void SetAngle_prog (tAxis ax, float angl);
	
			// установки максимальный скоростей перемещени€
			void SetEnable_Border (tAxis ax, bool f_val);
			void SetEnable_P (tAxis ax, bool f_val);
			void SetEnable_I (tAxis ax, bool f_val);
			void SetEnable_D (tAxis ax, bool f_val);
			void SetValue_P (tAxis ax, float val);
			void SetValue_I (tAxis ax, float val);
			void SetValue_D (tAxis ax, float val);
			void SetTimeStep_I (tAxis ax, uint32_t vall);
			void SetTimeOff_I (tAxis ax, uint32_t vall);
			void SetTimeRelax_I (tAxis ax, uint32_t vall);
			void SetEnableAxis (tAxis ax, bool f_val);
	
			void SetMotorPower (tAxis ax, uint16_t pwrval);
			void SetMotorPoles (tAxis ax, uint16_t poles);
			void SetAnglOffset (tAxis ax, float angl);
			void SetBordersAngl (tAxis ax, float angl_min, float angl_max);		
			void SetInverseAnglMotor (tAxis ax, bool inv);	
			void SetInverseAnglSens (tAxis ax, bool inv);
			//void SetPIDs (tAxis ax, float p, float i, float d);

};



#endif
