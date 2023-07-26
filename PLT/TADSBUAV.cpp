#include "TADSBUAV.H"

extern void canardEnSclr(void* destination, uint32_t &bit_offset, uint8_t bit_length, const void* value);


TADSBUAV::TADSBUAV ()
{
	f_new_adsb_data_usart = false;
	AddObjectToExecuteManager ();
	f_new_adsb_data_uavcan = false;			// потом удалить
	typeletun = 1;
}



void TADSBUAV::Init ()
{
lat = 464824235;
data.f_baro_valid = 1;
data.f_lat_lon_valid = 1;
data.f_heading_valid = 1;
data.f_velocity_valid = 1;
data.f_callsign_valid = 1;
data.f_ident_valid = 1;
data.f_simulated_report = 1;
data.f_vertical_velocity_valid = 1;
data.icao_address = 0x4D2189;
data.alt_m = 800;
data.squawk = 640;
data.callsign[0] = 'L';
data.callsign[1] = 'E';
data.callsign[2] = 'T';
data.callsign[3] = 'I';
data.callsign[4] = 'P';
data.callsign[5] = 'T';
data.callsign[6] = 'I';
data.callsign[7] = 'C';
data.callsign[8] = 'A';
data.latitude_deg_1e7 = lat;//464824235;
data.longitude_deg_1e7 = 305235423 ;//305235423;
data.traffic_type = typeletun;//ETRAFFIC_T_LARGE;//ETRAFFIC_T_UAV;//ETRAFFIC_T_GLIDER;//ETRAFFIC_T_UAV;
data.source = ESOURCE_T_ADSB;//ETRAFFIC_T_HEAVY;//ESOURCE_T_ADSB;//ESOURCE_T_ADSB;//ESOURCE_T_ADSB_UAT;
data.velocity[0] = 10;
data.velocity[1] = 11;
data.velocity[2] = 2;
data.alt_type = ETRAFFIC_T_UNKNOWN;// EALT_T_WGS84;//EALT_T_PRESSURE_AMSL;
data.tslc = 2;
data.heading = heatting;

}



void TADSBUAV::Task ()
{
	if (f_new_adsb_data_usart)
		{	
		data.timestamp++;
		f_new_adsb_data_uavcan = true;
		f_new_adsb_data_usart = false;
		}
}



uavcan_equipment_adsb *TADSBUAV::GetData ()
{
	uavcan_equipment_adsb *rv = 0;
	if (true)  // f_new_adsb_data_uavcan
		{
		rv = &data;
		lat += 1;
		heatting += 1;	
		data.latitude_deg_1e7 = lat;//464824235;
		data.heading = heatting;
		data.traffic_type = typeletun++;
		if (typeletun > 19) typeletun = 1;
			
		//f_new_adsb_data_uavcan = false; // раскоментировать после 
		}
	return rv;
}

