#ifndef _H_TADSBUAVCAN_STM32_H_
#define _H_TADSBUAVCAN_STM32_H_

#include "TFTASKIF.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "rutine.h"


//ardupilot.equipment.trafficmonitor.TrafficReport
#define UAVCAN_EQU_MON_TRAFFIC 											    (0x68e45db60b6981f8ULL)	


enum EALT_T{
EALT_T_UNKNOWN = 0, \
EALT_T_PRESSURE_AMSL = 1, \
EALT_T_WGS84 = 2
};


enum ESOURCE_T {
ESOURCE_T_ADSB = 0, \
ESOURCE_T_ADSB_UAT = 1, \
ESOURCE_T_FLARM = 2
};


enum ETRAFFIC_T{
ETRAFFIC_T_UNKNOWN = 0, \
ETRAFFIC_T_LIGHT = 1, \
ETRAFFIC_T_SMALL = 2, \
ETRAFFIC_T_LARGE = 3, \
ETRAFFIC_T_HIGH_VORTEX_LARGE = 4, \
ETRAFFIC_T_HEAVY = 5, \
ETRAFFIC_T_HIGHLY_MANUV = 6, \
ETRAFFIC_T_ROTOCRAFT = 7, \
ETRAFFIC_T_GLIDER = 9, \
ETRAFFIC_T_LIGHTER_THAN_AIR = 10, \
ETRAFFIC_T_PARACHUTE = 11, \
ETRAFFIC_T_ULTRA_LIGHT = 12, \
ETRAFFIC_T_UAV = 14, \
ETRAFFIC_T_SPACE = 15, \
ETRAFFIC_T_EMERGENCY_SURFACE = 17, \
ETRAFFIC_T_SERVICE_SURFACE = 18, \
ETRAFFIC_T_POINT_OBSTACLE = 19 \
};


typedef struct {
	uint64_t timestamp;
	unsigned long icao_address;				// 32
	unsigned short tslc;						// 16
	unsigned long latitude_deg_1e7;				// 32
	unsigned long longitude_deg_1e7;				// 32
	float alt_m;									// 32
	float heading;								// 16
	float velocity[3];						// 16 
	unsigned short squawk;				// 16
	unsigned char callsign[9];
	unsigned char source;
	unsigned char traffic_type;
	unsigned char alt_type;
	
	bool f_lat_lon_valid;
	bool f_heading_valid;
	bool f_velocity_valid;
	bool f_callsign_valid;
	bool f_ident_valid;
	bool f_simulated_report;
	bool f_vertical_velocity_valid;
	bool f_baro_valid;
	
} uavcan_equipment_adsb;

/*
uavcan.Timestamp timestamp
saturated uint32 icao_address
saturated uint16 tslc
saturated int32 latitude_deg_1e7
saturated int32 longitude_deg_1e7
saturated float32 alt_m
saturated float16 heading
saturated float16[3] velocity
saturated uint16 squawk
saturated uint8[9] callsign
saturated uint3 source
saturated uint5 traffic_type
saturated uint7 alt_type

saturated bool lat_lon_valid
saturated bool heading_valid
saturated bool velocity_valid
saturated bool callsign_valid
saturated bool ident_valid
saturated bool simulated_report
saturated bool vertical_velocity_valid
saturated bool baro_valid
*/


class TADSBUAV: public TFFC {
	
		uavcan_equipment_adsb data;
	
		//BUFPAR data_complete;
		
		//uavcan_equipment_adsb *GetData ();		// STRAFICDATA *frm
		virtual void Task ();
	
		bool f_new_adsb_data_usart;
		bool f_new_adsb_data_uavcan;
	
		unsigned long lat;
		unsigned short heatting;
		unsigned char typeletun;
	
	public:
		TADSBUAV ();
		void Init ();
	
		uavcan_equipment_adsb *GetData ();		// STRAFICDATA *frm

};


#endif

