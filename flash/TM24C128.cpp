#include "TM24C128.h"



TM24CIF::TM24CIF (TI2CIFACE *i2, uint8_t csa) : chip_sel_adr (csa)
{
	i2c = i2;
}



TM24C128::TM24C128 (TI2CIFACE *i2, uint8_t csa) : TM24CIF(i2, csa)
{
	page_contrl_mask = 0x8040;
	c_mem_size = 16384;
}



bool TM24C128::write_page (uint16_t adr, uint8_t *src, uint16_t sz_wr, uint16_t &rslt_wr)
{
	bool rv = false;
	uint16_t wr_cnt = 0, adr_chng = adr & 0x8040;
	bool rslt = false;
	if (sz_wr)
		{
		i2c->Start_I2C ();
		do	{
				if (!i2c->DataOut_I2C (chip_sel_adr)) break;
				if (!i2c->DataOut_I2C (adr >> 8)) break;
				if (!i2c->DataOut_I2C (adr)) break;
				rv = true;
				while (sz_wr)
					{
					if (!i2c->DataOut_I2C (*src++)) 
						{
						rv = false;
						break;
						}
					adr++;
					if (adr >= c_mem_size)
						{
						rv = false;
						break;
						}
					wr_cnt++;
					if ((adr & 0x8040) != adr_chng) break;
					sz_wr--;
					}
			if (rv) rslt_wr = wr_cnt;
			} while (false);
		i2c->Stop_I2C ();
		}
	return rv;
}



bool TM24C128::write (uint16_t adr, uint8_t *src, uint16_t sz)
{
	bool rv = false;
	uint16_t szwr_rslt;
	while (sz)
		{
		if (!write_page (adr, src, sz, szwr_rslt)) break;
		if (szwr_rslt == sz)
			{
			rv = true;
			break;
			}
		adr += szwr_rslt;
		src += szwr_rslt;
		sz -= szwr_rslt;
		}
	return rv;

}



bool TM24C128::read (uint16_t adr, uint8_t *dst, uint16_t sz)
{
	return i2c->ReadFrame_i2c (chip_sel_adr, adr, dst, sz);
}


