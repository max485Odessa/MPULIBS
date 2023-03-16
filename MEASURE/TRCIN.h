#ifndef _H_RCIN_H_
#define _H_RCIN_H_


#include "definitions.h"
#include <stdint.h>
#include "TFTASKIF.h"
#include "sysbios.h"



#define RC2STEP 0.002F

class IRCIN {
	public:
		virtual void Get_RC_Step (float *Step) = 0;
};




class TRCIN: public TFFC, public IRCIN {
		TRCIN ();
		static TRCIN *singlobj;
		float RCSmooth[NUMAXIS];
		
		long GetAUX3 ();
		long GetAUX4 ();
		long GetAUX2 ();
	
		virtual void Task ();
		virtual void Get_RC_Step (float *Step);		// диапазон выходных величин от -500.0 до 500.0
	
	protected:
	
	public:
		static TRCIN *obj ();
		static IRCIN *obj_usr ();
		void init ();
};


#endif
