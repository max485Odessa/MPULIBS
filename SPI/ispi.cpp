/*
 * ispi.cpp
 *
 *  Created on: Feb 10, 2025
 *      Author: Maxim
 */

#include "ispi.hpp"



TSOFTSPI::TSOFTSPI (S_INITPINS_P *s)
{
	pins = s;
	hard_init (false);
}



void TSOFTSPI::thizif_hiz_outputs (bool f_act_hiz)
{
	hard_init (f_act_hiz);
}



void TSOFTSPI::hard_init (bool f_act_hiz)
{
	_pin_low_init_in (&pins->miso, 1, EHRTGPIOSPEED_HI, EHRTGPIOPULL_NO);

	if (!f_act_hiz)
		{
		_pin_low_init_out_pp (pins->u.arr, 3, EHRTGPIOSPEED_HI);
		}
	else
		{
		_pin_low_init_in (pins->u.arr, 3, EHRTGPIOSPEED_HI, EHRTGPIOPULL_NO);
		}
}



void TSOFTSPI::cs_0 ()
{
	pins->u.name.cs.port->BSRR = ((uint32_t)pins->u.name.cs.pin) << 16;
}



void TSOFTSPI::cs_1 ()
{
	pins->u.name.cs.port->BSRR = pins->u.name.cs.pin;
}



uint8_t TSOFTSPI::txrx (uint8_t byte)
{
unsigned char rv = 0, cnt = 8;
	while (cnt)
		{
		rv <<= 1;
		if (byte & 128)
			{
			pins->u.name.mosi.port->BSRR = pins->u.name.mosi.pin;
			}
		else
			{
			pins->u.name.mosi.port->BSRR = ((uint32_t)pins->u.name.mosi.pin) << 16;
			}
		pins->u.name.sck.port->BSRR = pins->u.name.sck.pin;
		if (pins->miso.port->IDR & pins->miso.pin) rv |= 1;
		pins->u.name.sck.port->BSRR = ((uint32_t)pins->u.name.sck.pin) << 16;
		byte <<= 1;
		cnt--;
		}
return rv;
}



void TSOFTSPI::init ()
{
	hard_init (false);
}




void TSOFTSPI::deinit ()
{
	hard_init (true);
}


