#ifndef _H_FIFO_TEMPLATE_H_
#define _H_FIFO_TEMPLATE_H_

#include <stdint.h>

template<class Tp>
class TTFIFO {
	
		const uint32_t c_alloc_frames;
		uint32_t push_ix;
		uint32_t pop_ix;
		uint32_t fill_count;
		uint32_t peack_count;
		Tp *circlbuffer;
	
	public:
		TTFIFO (uint32_t el_count);	// елементный размер
		uint32_t is_free_space ();
		uint32_t frame_count ();
		uint32_t statistic_peak ();
		void statistic_peak_clear ();
		bool push (const Tp *frm);
		bool push (Tp frm);
		bool push (const Tp *frm, uint32_t sz);
		bool pop (Tp &frm);
		uint32_t pop (Tp *frm, uint32_t maxsz);
		bool peek (Tp &frm);
		void clear ();
		
};



template <class Tp>
TTFIFO<Tp>::TTFIFO (uint32_t el_count) : c_alloc_frames (el_count)
{
circlbuffer = new Tp[el_count];
clear();
statistic_peak_clear ();
}


template <class Tp>
uint32_t TTFIFO<Tp>::statistic_peak ()
{
	return peack_count;
}



template <class Tp>
void TTFIFO<Tp>::statistic_peak_clear ()
{
	peack_count = 0;
}



template <class Tp>
void TTFIFO<Tp>::clear ()
{
	push_ix = 0;
	pop_ix = 0;
	fill_count = 0;
}



template <class Tp>
uint32_t TTFIFO<Tp>::frame_count ()
{
	return fill_count;
}



template <class Tp>
uint32_t TTFIFO<Tp>::is_free_space ()
{
return c_alloc_frames - fill_count;	
}

template <class Tp>
bool TTFIFO<Tp>::push (const Tp *frm)
{
	bool rv = false;
	if (is_free_space () && frm)
		{
		circlbuffer[push_ix++] = *frm;
		if (push_ix >= c_alloc_frames) push_ix = 0;
		fill_count++;
		if (fill_count > peack_count) peack_count = fill_count;		// statistick
		rv = true;
		}
	return rv;
}

template <class Tp>
bool TTFIFO<Tp>::push (Tp frm)
{
	return push (&frm);
}

template <class Tp>
bool TTFIFO<Tp>::push (const Tp *frm, uint32_t sz)
{
	bool rv = false;
	while (sz)
		{
		if (!push (frm)) break;
		frm++;
		sz--;
		}
	if (!sz) rv = true;
	return rv;
}

template <class Tp>
bool TTFIFO<Tp>::pop (Tp &frm)
{
bool rv = false;
if (fill_count)
	{
	frm = circlbuffer[pop_ix++];
	if (pop_ix >= c_alloc_frames) pop_ix = 0;
	fill_count--;
	rv = true;
	}	
return rv;	
}

template <class Tp>
uint32_t TTFIFO<Tp>::pop (Tp *frm, uint32_t maxsz)
{
uint32_t rv = 0;
while (maxsz)
	{
	if (!pop (*frm++)) break;
	maxsz--;
	rv++;
	}
return rv;
}


template <class Tp>
bool TTFIFO<Tp>::peek (Tp &frm)
{
bool rv = false;
if (fill_count)
	{
	//if (pop_ix >= c_alloc_frames) pop_ix = 0;
	frm = circlbuffer[pop_ix];
	rv = true;
	}	
return rv;	
}



#endif
