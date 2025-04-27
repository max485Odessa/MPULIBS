/*
 * TEXECTASK.h
 *
 *  Created on: Mar 10, 2025
 *      Author: Maxim
 */

#ifndef SRC_TEXECTASK_HPP_
#define SRC_TEXECTASK_HPP_

#include <stdint.h>


class TFRDEXEC {
	public:
		virtual void task () = 0;
};


class TEXECTASK {
	TFRDEXEC **arr;
	const uint8_t c_maxsz;
	uint8_t add_ix;

public:
	TEXECTASK (uint8_t cnt);
	void add (TFRDEXEC *p);
	void execute ();
};


#endif /* SRC_TEXECTASK_HPP_ */
