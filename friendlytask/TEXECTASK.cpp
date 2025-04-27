/*
 * TEXECTASK.cpp
 *
 *  Created on: Mar 10, 2025
 *      Author: Maxim
 */

#include "TEXECTASK.hpp"


TEXECTASK::TEXECTASK (uint8_t cnt) : c_maxsz (cnt)
{
	arr = new TFRDEXEC* [c_maxsz];
	add_ix = 0;
}



void TEXECTASK::add (TFRDEXEC *p)
{
	if (p && add_ix < c_maxsz) arr[add_ix++] = p;
}



void TEXECTASK::execute ()
{
uint8_t ix = 0;
while (ix < add_ix)
	{
	arr[ix]->task ();
	ix++;
	}
}


