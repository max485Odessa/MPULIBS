#ifndef _H_PPM_BUS_CLASS_H_
#define _H_PPM_BUS_CLASS_H_

#include "TFTASKIF.h"
#include "SYSBIOS.H"
#include "rutine.h"
#include "TSBUS_UAV.H"	// дефайн TSBUSDATA 	
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_exti.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_tim_ex.h"


#define C_PPM_PIN_RX GPIO_PIN_15
#define C_PPM_PORT GPIOA



#define C_EXTI915AMOUNT 7
#define C_PPMVALUE_MAX 2200
#define C_PPMVALUE_MIN 800
#define C_PPMINTERVALBETW_MIN 8000
#define C_PPMINTERVALBETW_MAX 20000


class TEXTIIFACE {
	protected:
		static unsigned long proc_cnt;
		static TEXTIIFACE *ifacearray[C_EXTI915AMOUNT];
		bool add_cb (TEXTIIFACE *obj);
		long find_cb (TEXTIIFACE *obj);
		EXTI_HandleTypeDef hexti_hall;
		EXTI_ConfigTypeDef exti_conf;
		IRQn_Type nvic_irq;
		void NVIC_ENABLE (bool stat);
	
	public:
		void isr ();
		virtual void Pending_cb ();
};



class TEXTIMANAGER: public TEXTIIFACE {
	protected:

	public:
		static void ISRs ();	
};



#define C_PPMCHANEL_AMOUNT 8
typedef struct {
	unsigned long array[C_PPMCHANEL_AMOUNT];
} SPPMARRAYDATA;


// 15 мс межпакетный интервал(пасивное состояние лог 1), 9 импульсов(лог 0), 8 каналов, 350 микросекунд импульс.

class TPPMBUS : public TFFC, public TEXTIIFACE, public TSBUSIFACE {
	
	private:
		
		TIM_HandleTypeDef TimHandle;
	
		unsigned long prevval_tim;
		SPPMARRAYDATA raw_array;
		unsigned long add_chan_ix;
	
		SPPMARRAYDATA results_array;
		bool f_meas_complete;
	
	
		TSBUSDATA sbus_data;
		

	public:
		TPPMBUS ();
		virtual void Task ();
		virtual void Init ();
		virtual void Pending_cb ();
	
		virtual TSBUSDATA *GetData ();
		//void UnBlockAfter_recv ();

};



#endif

