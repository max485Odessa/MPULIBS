#include "uav_charger_cmd.h"
#include "rutine.h"


uint32_t uav_spaibase_cmd_encode (uav_equipment_spaibase_cmd_t *src, void* msg_buf, uint32_t offset)
{

	canardEnSclr (msg_buf, offset, 8, (void*)&src->cmd);		// cmd
	canardEnSclr (msg_buf, offset, 8, (void*)&src->param);	// param
	
	uint8_t ix = 0;
	while (ix < C_SPAISERIAL_LENGHT)
		{
		canardEnSclr (msg_buf, offset, 8, (void*)&src->src_id[ix]);		// src sn id
		ix++;
		}
		
	ix = 0;
	while (ix < C_SPAISERIAL_LENGHT)
		{
		canardEnSclr (msg_buf, offset, 8, (void*)&src->dst_id[ix]);		// dst sn id
		ix++;
		}
	
	return (offset + 7 ) / 8;
}



int32_t uav_spaibase_cmd_decode (const CanardRxTransfer* transfer,  uav_equipment_spaibase_cmd_t *dest)
{
	int32_t ret = -1;
	int32_t ofs_rv = 0, ix;
	
	memset (dest, 0, sizeof(uav_equipment_spaibase_cmd_t));
	
	do	{
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->cmd) <= 0) break;
			ofs_rv += 8;
			
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->param) <= 0) break;
			ofs_rv += 8;
				
			ix = 0;
			while (ix < C_SPAISERIAL_LENGHT)
				{
				if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->src_id[ix]) <= 0) break;
				ofs_rv += 8;
				ix++;
				}
			if (ix != C_SPAISERIAL_LENGHT) break;
		
			ix = 0;
			while (ix < C_SPAISERIAL_LENGHT)
				{
				if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->dst_id[ix]) <= 0) break;
				ofs_rv += 8;
				ix++;
				}
			if (ix != C_SPAISERIAL_LENGHT) break;
				
			ret = ofs_rv;
			} while (false);
	
	return ret;
}




uint32_t uav_equipment_sbattery_full_data_encode (uav_equipment_sbattery_full_data_t *src, void* msg_buf, uint32_t offset)
{

	
	canardEnSclr (msg_buf, offset, 8, (void*)&src->cmd);		// cmd
	canardEnSclr (msg_buf, offset, 8, (void*)&src->param);	// param
	
	uint8_t ix = 0;
	while (ix < C_SPAISERIAL_LENGHT)
		{
		canardEnSclr (msg_buf, offset, 8, (void*)&src->src_id[ix]);		// src sn id
		ix++;
		}
		
	ix = 0;
	while (ix < C_SPAISERIAL_LENGHT)
		{
		canardEnSclr (msg_buf, offset, 8, (void*)&src->dst_id[ix]);		// dst sn id
		ix++;
		}
	
	canardEnSclr (msg_buf, offset, 8, (void*)&src->bat_model);
	canardEnSclr (msg_buf, offset, 16, (void*)&src->c_volt);
	canardEnSclr (msg_buf, offset, 16, (void*)&src->c_current);
	canardEnSclr (msg_buf, offset, 16, (void*)&src->now_volt);
	canardEnSclr (msg_buf, offset, 16, (void*)&src->now_cur);
		
	ix = 0;
	while (ix < C_SBAT_CELL_MAXIMAL)
		{
		canardEnSclr (msg_buf, offset, 8, (void*)&src->cells_volt[ix]);
		ix++;
		}
	canardEnSclr (msg_buf, offset, 16, (void*)&src->cicles);
	canardEnSclr (msg_buf, offset, 16, (void*)&src->local_time_1s_lsb);
		
	canardEnSclr (msg_buf, offset, 8, (void*)&src->temp);
	
	return (offset + 7 ) / 8;
}



int32_t uav_equipment_sbattery_full_data_decode (const CanardRxTransfer* transfer, uav_equipment_sbattery_full_data_t *dest)
{
	int32_t ret = -1;
	int32_t ofs_rv = 0, ix;
	
	memset (dest, 0, sizeof(uav_equipment_spaibase_cmd_t));
	
	do	{
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->cmd) <= 0) break;
			ofs_rv += 8;
			
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->param) <= 0) break;
			ofs_rv += 8;
				
			ix = 0;
			while (ix < C_SPAISERIAL_LENGHT)
				{
				if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->src_id[ix]) <= 0) break;
				ofs_rv += 8;
				ix++;
				}
			if (ix != C_SPAISERIAL_LENGHT) break;
		
			ix = 0;
			while (ix < C_SPAISERIAL_LENGHT)
				{
				if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->dst_id[ix]) <= 0) break;
				ofs_rv += 8;
				ix++;
				}
			if (ix != C_SPAISERIAL_LENGHT) break;
				
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->bat_model) <= 0) break;
			ofs_rv += 8;
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->c_volt) <= 0) break;
			ofs_rv += 16;
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->c_current) <= 0) break;
			ofs_rv += 16;
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->now_volt) <= 0) break;
			ofs_rv += 16;
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->now_cur)<= 0 ) break;
			ofs_rv += 16;
				
			ix = 0;
			while (ix < C_SBAT_CELL_MAXIMAL)
				{
				if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->cells_volt[ix]) <= 0) break;
				ofs_rv += 8;
				ix++;
				}
			if (ix != C_SBAT_CELL_MAXIMAL) break;
				
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->cicles) <= 0) break;
			ofs_rv += 16;
			if (canardDecodeScalar (transfer, ofs_rv, 16, false, (void*)&dest->local_time_1s_lsb) <= 0) break;
			ofs_rv += 16;	
			if (canardDecodeScalar (transfer, ofs_rv, 8, false, (void*)&dest->temp) <= 0) break;
			ofs_rv += 8;
				
			ret = ofs_rv;
			} while (false);
	return ret;
}




