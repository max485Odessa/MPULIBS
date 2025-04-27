/*
 * diagnosticif.hpp
 *
 *  Created on: Apr 7, 2025
 *      Author: Maxim
 */

#ifndef SRC_DIAGNOSTICIF_HPP_
#define SRC_DIAGNOSTICIF_HPP_

#include "SYSBIOS.hpp"



class TDIAGIF {
	public:
		enum ESTATE {ESTATE_NONE = 0, ESTATE_WAIT, ESTATE_OK, ESTATE_ERROR, ESTATE_ENDENUM};

		virtual void diagnostick_start (uint32_t time_max) = 0;
		virtual ESTATE diagnostick_process (char **txtout) = 0;
		ESTATE last_disgnostic_state ();

		const char *diagnostick_device_name ();

	protected:
		SYSBIOS::Timer diag_timer;
		bool f_diagnostic_active;
		char *diagnostick_rslt_txt;
		ESTATE diagnostick_last_state;
		TDIAGIF (const char *dnm);

		const char *devname_txt;

};



#endif /* SRC_DIAGNOSTICIF_HPP_ */
