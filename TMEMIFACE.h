#ifndef _H_MEMORY_IFACE_H_
#define _H_MEMORY_IFACE_H_



#include <stdint.h>
#include "DISPATCHIFACE.H"
#include "TGBLCMD.h"




#define C_M24C02I2CADR 0x50



typedef struct {
	union {
				float data_f;
				uint32_t data_l;
				};
} S_MEMPARAMTAG_T;



class TMEMIF {
	protected:
		bool f_data_is_changed;
		S_MEMPARAMTAG_T memtags[EGBLPARAM_ENDENUM];
		virtual bool write_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T *tag) = 0;
		virtual bool read_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T &tag) = 0;
	
	public:
		virtual bool WriteData_l (EGBLPARAM parix, uint32_t data) = 0;
		virtual bool WriteData_f (EGBLPARAM parix, float data) = 0;
		virtual bool ReadData_l (EGBLPARAM parix, uint32_t &data) = 0;
		virtual bool ReadData_f (EGBLPARAM parix, float &data) = 0;
		virtual uint32_t ReadData_l (EGBLPARAM parix) = 0;
		virtual float ReadData_f (EGBLPARAM parix) = 0;
		void load_all (TI2CIFACE *i2cobj);
		bool is_changed ();
};




enum E_24C02ST {E_24C02ST_INIT_LOAD = 0, E_24C02ST_INIT, E_24C02ST_CHK_NEED_WR, E_24C02ST_WR, E_24C02ST_CHK_WR, E_24C02ST_BUSSERROR, E_24C02ST_RELEASE, E_24C02ST_ENDENUM};

class T24C02MEM : public TI2CTRDIFACE, public TMEMIF {
	
		virtual void Start ();
		virtual void Task (TI2CIFACE *i2cobj);
		static T24C02MEM *singltn;
		E_24C02ST state_sw;
	
		
		bool memchangflags[EGBLPARAM_ENDENUM];
		void clear_change_flags ();
	
		utimer_t timer_autosave;
	
		uint32_t chk_wr_ix;
		
	
		long get_needwr_data_ix ();
		uint32_t cur_wr_ix;
	
		virtual bool write_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T *tag);
		virtual bool read_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T &tag);
		
		virtual bool WriteData_l (EGBLPARAM parix, uint32_t data);
		virtual bool WriteData_f (EGBLPARAM parix, float data);
		virtual bool ReadData_l (EGBLPARAM parix, uint32_t &data);
		virtual bool ReadData_f (EGBLPARAM parix, float &data);
		virtual uint32_t ReadData_l (EGBLPARAM parix);
		virtual float ReadData_f (EGBLPARAM parix);
	
		T24C02MEM ();

	public:
		static T24C02MEM *obj ();
		static TMEMIF *mem_ifc ();
};



#endif
