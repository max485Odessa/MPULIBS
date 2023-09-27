#include "TCDC.h"


extern "C" uint8_t tusbobj_tx_data (uint8_t *d);

#ifdef __cplusplus
 extern "C" {
#endif 
	 
extern PCD_HandleTypeDef hpcd;
USBD_HandleTypeDef USBD_Device;
extern uint16_t cdc_tx_buffer_free_space ();
	 
void tusbobj_rx_data (uint8_t* Buf, uint32_t Len)
{
	TSERIALISR &usb = *TSERIALISR::ifc[ESYSUSB_1];
	
	while (Len)
		{
		usb.isr_rx (*Buf++);
		Len--;
		}
}



void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd);
}


uint8_t tusbobj_tx_data (uint8_t *d)
{
	uint8_t rv = 0, data;
	TSERIALISR &usb = *TSERIALISR::ifc[ESYSUSB_1];
	rv = usb.isr_tx (data);
	if (rv) *d = data;
	return rv;
}


void tusbobj_reconnect ()
{
TUSBSOME::reconnect_signal ();
}


void tusbobj_baudrate (uint32_t br)
{
	TUSBSOME::set_baudrate (br);
}





#ifdef __cplusplus
}
#endif


SYSBIOS::Timer TUSBSOME::link_timer;
uint32_t TUSBSOME::c_link_period = 1000;
bool TUSBSOME::f_reconnect_flag = false;
uint32_t TUSBSOME::baudrate_set = 0;
bool TUSBSOME::f_config_flag = false;


void TUSBSOME::reconnect_signal ()
{
	f_reconnect_flag = true;
}



void TUSBSOME::set_baudrate (uint32_t br)
{
	baudrate_set = br;
	f_config_flag = true;
}



TUSBOBJ::TUSBOBJ (uint32_t sz_b_tx, uint32_t sz_b_rx)
{
	ifc[ESYSUSB_1] = this;
	fifo_tx = new TTFIFO<uint8_t>(sz_b_tx);
	fifo_rx = new TTFIFO<uint8_t>(sz_b_rx);
	c_link_period = 1000;		// 1s

	init_hardware ();
}



bool TUSBOBJ::config_detect ()
{
	bool rv = f_config_flag;
	if (f_config_flag) f_config_flag = false;
	return rv;
}



bool TUSBOBJ::reconnect_detected ()
{
	bool rv = f_reconnect_flag;
	if (f_reconnect_flag) f_reconnect_flag = false;
	return rv;
}



bool TUSBOBJ::is_link ()
{
	return (link_timer.get())?true:false;
}



void TUSBOBJ::set_link_period (uint32_t tm)
{
	c_link_period = tm;
}



uint32_t TUSBOBJ::get_baudrate ()
{
	return baudrate_set;
}



void TUSBOBJ::init_hardware ()
{
  USBD_Init (&USBD_Device, &VCP_Desc, 0);
  USBD_RegisterClass (&USBD_Device, USBD_CDC_CLASS);
  USBD_CDC_RegisterInterface (&USBD_Device, &USBD_CDC_fops);
  USBD_Start (&USBD_Device);
}



void TUSBOBJ::isr_rx (uint8_t d)
{
	link_timer.set (c_link_period);		// link control
	fifo_rx->push (d);
}



bool TUSBOBJ::isr_tx (uint8_t &d)
{
	f_tx_status = fifo_tx->pop (d);
	return f_tx_status;
}



bool TUSBOBJ::Tx (void *lTx, uint16_t sz_tx)
{
	bool rv = fifo_tx->push ((uint8_t *)lTx, sz_tx);
	if (rv) f_tx_status = true;
	return rv;
}



bool TUSBOBJ::Tx_status ()
{
	return f_tx_status;
}



uint16_t TUSBOBJ::Rx_check ()
{
	return fifo_rx->frame_count ();
}



uint16_t TUSBOBJ::Rx (void *lDst, uint16_t max_size)
{
	return fifo_rx->pop ((uint8_t *)lDst, max_size);
}


