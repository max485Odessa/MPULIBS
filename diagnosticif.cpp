/*
 * diagnosticif.cpp
 *
 *  Created on: Apr 7, 2025
 *      Author: Maxim
 */

#include "diagnosticif.hpp"

static const char *ltxt_none = "";


TDIAGIF::TDIAGIF (const char *dnm) : devname_txt (dnm)
{
	diagnostick_rslt_txt = (char*)ltxt_none;
	diagnostick_last_state = TDIAGIF::ESTATE_NONE;
	f_diagnostic_active = false;
	diag_timer.set(0);
}



TDIAGIF::ESTATE TDIAGIF::last_disgnostic_state ()
{
	return diagnostick_last_state;
}



const char *TDIAGIF::diagnostick_device_name ()
{
	return devname_txt;
}

