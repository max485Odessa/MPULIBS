#ifndef _h_easy_storage_interface_implenent_hal_stm32f103_h_
#define _h_easy_storage_interface_implenent_hal_stm32f103_h_


#include "STORAGEIF.h"
#include "hard_rut.h"


class TSTORGF103: public IFSTORAGE {
		const uint32_t c_page_start_adr;
		const uint32_t c_page_stop_adr;
		virtual uint32_t file_size ();
		virtual bool write (uint32_t adrix, uint8_t *src, uint32_t wr_size);
		virtual bool read (uint32_t adrix, uint8_t *dst, uint32_t wr_size);
	
		uint32_t erase_pages (uint32_t start_adr, uint32_t stp_adr);
		void unlock_flash ();
		void lock_flash ();
		bool get_word (uint32_t c_adr, uint32_t last_adr, uint32_t &dstdata, uint8_t &lsize);
		bool get_word (uint8_t *adr, uint32_t cursize, uint32_t &dstdata, uint8_t &lsize);
	
	public:
		TSTORGF103 (uint32_t start_p, uint32_t stop_p);
		
};

#endif

