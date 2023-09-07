#ifndef _H_STM32F4_HALLLIB_M24128_H_
#define _H_STM32F4_HALLLIB_M24128_H_



#include "hard_rut.h"
#include "I2CSOFTWARE.H"



//enum EM24CXTYPE {EM24CXTYPE_NONE = 0, EM24CXTYPE_C16, EM24CXTYPE_C128, EM24CXTYPE_ENDENUM};



class TM24CIF {
	protected:
		TI2CIFACE *i2c;
		const uint8_t chip_sel_adr;		// adr + external sel pins
		uint16_t page_contrl_mask;
		uint16_t c_mem_size;
	
		virtual bool write_page (uint16_t adr, uint8_t *src, uint16_t sz_wr, uint16_t &rslt_wr) = 0;	// result bytes write from page
		TM24CIF (TI2CIFACE *i2, uint8_t csa);
	
	public:
		
		virtual bool write (uint16_t adr, uint8_t *src, uint16_t sz) = 0;
		virtual bool read (uint16_t adr, uint8_t *dst, uint16_t sz) = 0;
};



class TM24C128: public TM24CIF {

		virtual bool write (uint16_t adr, uint8_t *src, uint16_t sz) override;
		virtual bool read (uint16_t adr, uint8_t *dst, uint16_t sz) override;
		virtual bool write_page (uint16_t adr, uint8_t *src, uint16_t sz_wr, uint16_t &rslt_wr) override;	
	
	public:
		TM24C128 (TI2CIFACE *i2, uint8_t csa);
};



#endif

