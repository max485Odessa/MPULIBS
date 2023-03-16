#ifndef _H_MS4515_PRESSURE_STM32_H_
#define _H_MS4515_PRESSURE_STM32_H_



#include "DISPATCHIFACE.H"
#include "sysbios.h"
#include "rutine.h"


enum ESW4515 {ESW4515_INIT = 0, ESW4515_READ_A = 1, ESW4515_READ_B = 2, ESW4515_ERROR = 3, ESW4515_COMPLETE = 4};



class TMS4515: public TI2CTRDIFACE {
	
		uint8_t rawbuf_a[4];
		uint8_t rawbuf_b[4];
	
		const unsigned char C_SLAVEADRESS;
		const unsigned short C_MEASCYCLE;
		const unsigned short C_PERMIT_AMOUNT;
	
		ESW4515 Sw;
	
		bool f_new_parse_data_pres_temp;
		float dif_pressure_pascal;
		float temperature_kelvin;
		float offset_val_pascal;

	
		float _get_pressure (int16_t dp_raw) const;
		float _get_temperature (int16_t dT_raw) const;

		float get_psi_range() const;
		float c_value_psi_range;
		
		bool is_enabled;
		
	public:
		TMS4515 ();
		bool get_pressure_temp (float &pressv, float &tempv);
		void set_psi_range (float val);
		void set_pascal_offset (long vall);

		void enabled (bool val);
		
		virtual void Task (TI2CIFACE *i2cobj);
		virtual void Start ();
	
};


#endif

