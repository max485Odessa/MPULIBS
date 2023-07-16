#ifndef _H_PRESSURE_5V_SENSORS_H_
#define _H_PRESSURE_5V_SENSORS_H_


#include "TANALOGS.h"
#include "TPARAMS.h"



class TPRESSURE {
		const EASIG chnl;
		const float c_model_bar;
		IFAIN *volt;
		IRFPARAMS *params;
	public:
		TPRESSURE (IFAIN *ia, EASIG c, float b);
		bool pressure (float &dst);
		void calibrate_pressure_zero (float volt_zr);
		void calibrate_pressure_max (float volt_max);
};


#endif
