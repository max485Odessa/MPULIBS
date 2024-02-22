#ifndef _STM32F4xx_CDC_H_
#define _STM32F4xx_CDC_H_

#include "stm32f4xx_hal.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"
#include "tusartdata.h"
#include "TTFIFO.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern USBD_DescriptorsTypeDef VCP_Desc;
extern USBD_CDC_ItfTypeDef USBD_CDC_fops;

class TUSBSOME {
	protected:
		//static SYSBIOS::Timer link_timer;
		//static uint32_t c_link_period;
		static bool f_reconnect_flag;
		static bool f_config_flag;
		static uint32_t baudrate_set;
	
	public:
		static void reconnect_signal ();
		static void set_baudrate (uint32_t br);
};



class TUSBOBJ: public TSERIALISR, public TSERIALUSR, protected TUSBSOME {
		virtual void isr_rx (uint8_t d) override;
		virtual bool isr_tx (uint8_t &d) override;
	
		void init_hardware ();
		bool f_tx_status;
	
		virtual bool Tx (void *lTx, uint16_t sz_tx) override;
		virtual bool Tx_status () override;
	
		virtual uint16_t Rx_check () override;
		virtual uint16_t Rx (void *lDst, uint16_t max_size) override;
		
		
	public:
		TUSBOBJ (uint32_t sz_b_tx, uint32_t sz_b_rx);
		static bool reconnect_detected ();
		static bool config_detect ();
		virtual bool is_link () override;
		//static void set_link_period (uint32_t tm);
		static uint32_t get_baudrate ();
		
};



#endif
