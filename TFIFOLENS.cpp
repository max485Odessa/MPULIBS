#include "TFIFOLENS.h"



TFIFOLEN::TFIFOLEN (uint32_t sz)
{
	peack_frame_count = 0;
	item_cnt = 0;
	fifo = new TTFIFO<uint8_t>(sz);
}



bool TFIFOLEN::check_push_space (uint16_t sz)
{
	bool rv = false;
	if (fifo->is_free_space () >= (sz + sizeof(uint16_t))) rv = true;
	return rv;
}



void TFIFOLEN::clear ()
{
	fifo->clear ();
	item_cnt = 0;
}




bool TFIFOLEN::push (void *s, uint16_t sz)
{
	bool rv = check_push_space ( sz);
	if (rv && sz)
		{
		fifo->push ((sz >> 8) & 0xFF);
		fifo->push (sz & 0xFF);
		uint8_t *src8 = (uint8_t*)s;
		while (sz)
			{
			fifo->push (*src8++);
			sz--;
			}
		item_cnt++;
		if (item_cnt > peack_frame_count) peack_frame_count = item_cnt;
		}
	return rv;
}



bool TFIFOLEN::pop (void *d, uint16_t &sz_inmax_outcur)
{
	bool rv = false;
	if (!item_cnt)
		{
		if (fifo->frame_count ()) clear ();
		}
	else
		{
		if (fifo->frame_count () && sz_inmax_outcur)		// >= 3
			{
			uint8_t *dst8 = (uint8_t*)d;
			uint16_t sz_tag = 0;
			uint16_t rslt_sz;
			uint8_t data;
			do	{
					if (!fifo->pop (data)) break;
					sz_tag = data; sz_tag <<= 8;
					if (!fifo->pop (data)) break;
					sz_tag |= data;
					if (sz_tag > fifo->frame_count () || !sz_tag || sz_tag > sz_inmax_outcur) break;
					rslt_sz = sz_tag;
					while (sz_tag)
						{
						if (!fifo->pop (data)) break;
						*dst8++ = data;
						sz_tag--;
						}
					if (!sz_tag) 
						{
						item_cnt--;
						sz_inmax_outcur = rslt_sz;
						rv = true;
						}
					} while (false);
			}
		if (!rv) clear ();
		}
	return rv;
}



uint32_t TFIFOLEN::is_free_space ()
{
	uint32_t sz = fifo->is_free_space ();
	if (sz)
		{
		if (sz >= 3)
			{
			sz -= 2;
			}
		else
			{
			sz = 0;
			}
		}
	return sz;
}



uint32_t TFIFOLEN::frame_count ()
{
	return item_cnt;
}



uint32_t TFIFOLEN::statistic_frame_peack ()
{
	return peack_frame_count;
}



uint32_t TFIFOLEN::statistic_bytes_peack ()
{
	return fifo->statistic_peak ();
}



void TFIFOLEN::statistic_peack_clear ()
{
	fifo->statistic_peak_clear ();
	peack_frame_count = 0;
}

