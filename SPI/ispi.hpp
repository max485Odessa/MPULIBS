/*
 * ispi.h
 *
 *  Created on: Feb 10, 2025
 *      Author: Maxim
 */

#ifndef SRC_ISPI_H_
#define SRC_ISPI_H_


#include "hard_rut.h"
#include "thizif.h"



class ISPI {

public:
	typedef struct {
		S_GPIOPIN miso;
		union {
			struct {
				S_GPIOPIN sck;
				S_GPIOPIN mosi;
				S_GPIOPIN cs;
			} name;
			S_GPIOPIN arr[3];
		}u;

	} S_INITPINS_P;

	virtual void cs_0 () = 0;
	virtual void cs_1 () = 0;
	virtual uint8_t txrx (uint8_t dat) = 0;
	virtual void init () = 0;
	virtual void deinit () = 0;

};



class TSOFTSPI: public ISPI, public THIZIF {
public:
		TSOFTSPI (S_INITPINS_P *s);


	protected:
		S_INITPINS_P *pins;
		virtual void thizif_hiz_outputs (bool f_act_hiz) override;
		virtual void cs_0 () override;
		virtual void cs_1 () override;
		virtual uint8_t txrx (uint8_t dat) override;
		virtual void init () override;
		virtual void deinit () override;
		void hard_init (bool f_act_hiz);
};


#endif /* SRC_ISPI_H_ */
