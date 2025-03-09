#include "DISPATCHIFACE.hpp"


extern void debug_message (char *ltxt, unsigned char sz);
extern bool check_firmware_status ();



TI2CTRDIFACE::TI2CTRDIFACE (uint8_t adr) : C_SLAVEADRESS (adr)
{
	ClearError ();
	PermitCount = 0;
	F_complete = true;
	BusErrorCount_i2c = 0;
	BusErrorCount_chip = 0;
	BusErrorCount_measure = 0;
	f_need_ic_soft_reset = false;
	
	ReleaseTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&ReleaseTimeout);
}



unsigned long TI2CTRDIFACE::get_measure_error_cnt ()
{
	return BusErrorCount_measure;
}


unsigned long TI2CTRDIFACE::get_i2c_error_cnt ()
{
	return BusErrorCount_i2c;
}



unsigned long TI2CTRDIFACE::get_chip_error_cnt ()
{
	return BusErrorCount_chip;
}



const char *TI2CTRDIFACE::get_device_name ()
{
	return debug_chip_name;
}



void TI2CTRDIFACE::SetBussError_i2c ()
{
	BusErrorCount_i2c++;
	f_buss_error = true;
}



void TI2CTRDIFACE::SetBussError_chip ()
{
	BusErrorCount_chip++;
	f_chip_error = true;
}



void TI2CTRDIFACE::SetBussError_meas ()
{
	BusErrorCount_measure++;
	f_meass_error = true;
}



bool TI2CTRDIFACE::CheckComplete ()
{
return F_complete;
}



bool TI2CTRDIFACE::CheckError ()
{
	return f_buss_error | f_chip_error | f_meass_error;
}



void TI2CTRDIFACE::ClearError ()
{
	if (f_buss_error || f_chip_error || f_meass_error)
		{
		f_buss_error = false;
		f_chip_error = false;
		f_meass_error = false;
		f_need_ic_soft_reset = true;
		}
}



bool TI2CTRDIFACE::is_soft_reset ()
{
bool rv = f_need_ic_soft_reset;
if (f_need_ic_soft_reset) f_need_ic_soft_reset = false;
return rv;	
}



TDISPATCHI2C::TDISPATCHI2C (TI2CBUS *bus_obj, uint8_t c_maxi2c) : c_max_i2cdev (c_maxi2c)
{
	DevArray = new TI2CTRDIFACE*[c_max_i2cdev];
	I2C_device = bus_obj;
	DevCount = 0;
	CurDevIndx = 0;
	ReleaseTimeout = 0;
	debug_object = 0;
	SYSBIOS::ADD_TIMER_SYS (&ReleaseTimeout);
}



bool TDISPATCHI2C::Add (TI2CTRDIFACE *dev_iface)
{
	bool rv = false;
if (dev_iface && DevCount < c_max_i2cdev)
	{
	DevArray[DevCount++] = dev_iface;
	rv = true;
	}
return rv;
}



void TDISPATCHI2C::CircleIndexControl ()
{
	if (CurDevIndx > (DevCount - 1)) CurDevIndx = 0;
}



void TDISPATCHI2C::Task ()
{
	if (ReleaseTimeout) return;
	
	if (DevCount)
		{
		CircleIndexControl ();
		TI2CTRDIFACE *lIface = DevArray[CurDevIndx];
		if (lIface)
			{
			lIface->Task (I2C_device);	
				
			if (lIface->CheckComplete())
				{
				if (lIface->CheckError ())
					{
					debug_object = lIface;
					
					ReleaseTimeout = 2;
					lIface->ClearError ();	
					}
				else
					{

					CurDevIndx++;
					CircleIndexControl ();
					lIface = DevArray[CurDevIndx];
					if (lIface)
						{	
						lIface->Start ();
						lIface->Task (I2C_device);
						}
					else
						{
						CurDevIndx++;
						CircleIndexControl ();
						}
					}
				}
			}
		else
			{
			CurDevIndx++;
			CircleIndexControl ();
			}
		}
}



