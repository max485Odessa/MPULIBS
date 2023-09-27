#ifndef _H_FIFOLENS_H_
#define _H_FIFOLENS_H_


#include <stdint.h>
#include "TTFIFO.h"


class TFIFOLEN {
		TTFIFO<uint8_t> *fifo;
		uint32_t peack_frame_count;
		uint32_t item_cnt;
	
	public:
		TFIFOLEN (uint32_t sz);
		bool check_push_space (uint16_t sz);
		bool push (void *s, uint16_t sz);
		bool pop (void *d, uint16_t &sz);

		uint32_t frame_count ();
		uint32_t statistic_frame_peack ();
		uint32_t statistic_bytes_peack ();
		void statistic_peack_clear ();
		uint32_t is_free_space ();
		
};


#endif
