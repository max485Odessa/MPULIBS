#ifndef _H_PRESSURE_INTERFACE_H_
#define _H_PRESSURE_INTERFACE_H_


#include "DISPATCHIFACE.H"


class TPRSIFACE: public TI2CTRDIFACE {
	protected:
		float clbr_ofs_temp;
		long clbr_ofs_prs;
	
	public:
		
		void SetOffset_temp (float ofs);
		void SetOffset_prs (long ofs);
		
		virtual void Task () =0 ;
		virtual void Start () = 0;
		virtual void Config () = 0;
		virtual bool GetPressure (float &DataOut) = 0;
		virtual bool GetTemperature (float &DataOut) = 0;
		virtual void ClearFlagNewData () = 0;
		virtual bool CheckNewData () = 0;
};



#endif


