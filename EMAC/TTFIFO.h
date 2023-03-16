#ifndef _H_FIFO_TEMPLATE_H_
#define _H_FIFO_TEMPLATE_H_



template<class Tp>
class TTFIFO {
	
		const unsigned long c_alloc_frames;
		unsigned long push_ix;
		unsigned long pop_ix;
		unsigned long fill_count;
		Tp *circlbuffer;
	
	public:
		TTFIFO (unsigned long el_count);	// елементный размер
		unsigned long is_free_space ();
		unsigned long frame_count ();
		bool push (const Tp *frm);
		bool pop (Tp &frm);
		bool peek (Tp &frm);
		void clear ();
};



template <class Tp>
TTFIFO<Tp>::TTFIFO (unsigned long el_count) : c_alloc_frames (el_count)
{
circlbuffer = new Tp[el_count];
clear();
}



template <class Tp>
void TTFIFO<Tp>::clear ()
{
	push_ix = 0;
	pop_ix = 0;
	fill_count = 0;
}



template <class Tp>
unsigned long TTFIFO<Tp>::frame_count ()
{
	return fill_count;
}



template <class Tp>
unsigned long TTFIFO<Tp>::is_free_space ()
{
unsigned long rv = 0;
if (fill_count < c_alloc_frames) rv = fill_count;
return rv;	
}

template <class Tp>
bool TTFIFO<Tp>::push (const Tp *frm)
{
	bool rv = false;
	if (is_free_space () && frm)
		{
		if (push_ix >= c_alloc_frames) push_ix = 0;
		circlbuffer[push_ix++] = *frm;
		fill_count++;
		rv = true;
		}
	return rv;
}

template <class Tp>
bool TTFIFO<Tp>::pop (Tp &frm)
{
bool rv = false;
if (fill_count)
	{
	if (pop_ix >= c_alloc_frames) pop_ix = 0;
	frm = circlbuffer[pop_ix++];
	fill_count--;
	rv = true;
	}	
return rv;	
}

template <class Tp>
bool TTFIFO<Tp>::peek (Tp &frm)
{
bool rv = false;
if (fill_count)
	{
	if (pop_ix >= c_alloc_frames) pop_ix = 0;
	frm = circlbuffer[pop_ix];
	rv = true;
	}	
return rv;	
}



#endif
