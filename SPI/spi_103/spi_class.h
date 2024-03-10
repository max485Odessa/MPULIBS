#ifndef __SPI_COMON_CLASS_H_
#define __SPI_COMON_CLASS_H_


enum ESPISEL {ESPISEL_SPI1, ESPISEL_SPI2, ESPISEL_ENDENUM};


#include "hard_rut.h"
//#include "interfaces.h"
#include "spi_class_conf.h"


//#ifdef __cplusplus
 //extern "C" {
//#endif


//#ifdef __cplusplus
//}
//#endif






class ISPI {
		
	public:
		virtual void cs_0 () = 0;
		virtual void cs_1 () = 0;
		virtual uint8_t txrx (uint8_t dat) = 0;
		virtual void init () = 0;
		virtual void deinit () = 0;
};



class TSPI1HARD: public ISPI {
	private:
		bool F_Transaction;

		const S_SPIGPIO_T *spigpio;
		SPI_HandleTypeDef SpiHandle;
	
		virtual void cs_0 ();
		virtual void cs_1 ();
		virtual uint8_t txrx (uint8_t byte);
		virtual void init ();
		virtual void deinit ();
	
	protected:
		
		void Wait_cs (unsigned char wt_time);
		unsigned char Tech_wait_s, Tech_wait_p;
	
	public:
		TSPI1HARD (S_SPIGPIO_T *g);
		void csp_spi_read_fast (uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data);
		void csp_spi_write_fast (uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data);
	

};



/*
class TSPI: public ISPI {
		GPIO_TypeDef *mosi_port;
		GPIO_TypeDef *miso_port;
		GPIO_TypeDef *sck_port;
		GPIO_TypeDef *cs_port;
	
		uint16_t mosi_pin;
		uint16_t miso_pin;
		uint16_t sck_pin;
		uint16_t cs_pin;
	
	public:
		TSPI (ESPISEL s);
		virtual void cs_0 ();
		virtual void cs_1 ();
		virtual uint8_t txrx (uint8_t dat);
		
};
*/


#endif

