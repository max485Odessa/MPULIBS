#ifndef _H_EASY_MEMORY_STORAGE_H_
#define _H_EASY_MEMORY_STORAGE_H_


#include "winb25x16_stml4xx.h"
#include "rutine.h"


#define C_DEF_NAVINID_ADR 50

#define C_SBUS_RATE_MAX 1000
#define C_SBUS_RATE_DEF 20
#define C_SBUS_RATE_MIN 20

#define C_ADSB_RATE_MIN 300
#define C_ADSB_RATE_DEF 1000
#define C_ADSB_RATE_MAX 100000

#define C_ADSB_TIME_TEST_MIN 0
#define C_ADSB_TIME_TEST_MAX 120
#define C_ADSB_TIME_TEST_DEF 60


#define C_FIX_RATE_MIN 100
#define C_FIX_RATE_DEF 100
#define C_FIX_RATE_MAX 100000


#define C_AUX_RATE_MIN 1000
#define C_AUX_RATE_DEF 1000
#define C_AUX_RATE_MAX 100000


typedef struct {
	unsigned long arr_item;
} S_UAVPAR;


typedef struct {
	long temperature;
	unsigned short raw;
	unsigned char midl_acc_count;		// для статистики. Сколько раз усреднялась точка
	bool f_ok;
} SZEROPAIR;



// полная структура сохраняющая все калибровки offset
typedef struct {
	SZEROPAIR bot;
	SZEROPAIR top;
} STZOFTABLEFULL;


typedef void* TSPDATA;


enum ESAVEPARIX {ESAVEPARIX_NODE_ID = 0, ESAVEPARIX_SBUSORPPM_ON, ESAVEPARIX_SBUS_RATE, \
ESAVEPARIX_ADSB_ON, ESAVEPARIX_ADSB_RATE, \
ESAVEPARIX_FIX_ON, ESAVEPARIX_FIX_RATE, ESAVEPARIX_AUX_RATE, \
ESAVEPARIX_FIX_PRIO, ESAVEPARIX_AUX_PRIO, ESAVEPARIX_ADSB_PRIO, ESAVEPARIX_SBUS_PRIO, ESAVEPARIX_LINKCONTROL_ADR, \
ESAVEPARIX_ADSB_TEST_TIME, ESAVEPARIX_ENDENUM};

typedef struct {
	unsigned long Uav_self_id;			// свой адрес
	unsigned long SBUS_PPM_on;	
	unsigned long SBUS_rate;
	
	unsigned long ADSB_on;	
	unsigned long ADSB_rate;
	
	unsigned long GPS_on;	
	unsigned long GPS_fix_rate;
	unsigned long GPS_aux_rate;
	
	unsigned long gps_fix_prio;
	unsigned long gps_aux_prio;
	unsigned long adsb_prio;
	unsigned long sbus_prio;
	unsigned long link_control_adr;
	unsigned long adsb_test_time;
	
	unsigned long reserved[51];
	
}TPARAMFLASH;





typedef struct {
	TPARAMFLASH params;
	unsigned long CRC32;
} TPARAMSAVE;






#define C_FLASH_INI_ADRESS_A 0
#define C_FLASH_INI_ADRESS_B (C_FLASH_INI_ADRESS_A + sizeof(TPARAMSAVE))
#define C_FLASH_INI_ADRESS_C (C_FLASH_INI_ADRESS_B + sizeof(TPARAMSAVE))
#define C_UAVPARAM_COPY_AMOUNT 3
#define C_FLASH_FREE_ADR (C_FLASH_INI_ADRESS_C + sizeof(TPARAMSAVE))
	



class TEASYMEMSTORAGE {
	private:
		utimer_t Timer_release;
		static TPARAMSAVE data;
		static bool f_need_save;
		static TWINBOND25X16 *lFlash;
		static bool SaveToExtFlash (uint8_t inx_copy);
		static uint32_t GetArrAdr (uint8_t arr_indx);
		static void UpdateCRCAllList ();
		static uint32_t CalculateCRC (uint8_t *lSrc, uint32_t sz);
		static void Default_sets ();
		static bool f_bad_setings;
		static bool check_crc_from_flash (uint8_t inx_copy);
		static bool CheckCRCExtFlashCopy (unsigned char arr_indx);
		static bool LoadFromExtFlash (uint8_t inx_copy);
		//static CB_SAVE_PARAM lp_call_back_rutine;

	public:
		TEASYMEMSTORAGE ();
		void Init (TWINBOND25X16 *m);
		void Task ();
		
		static void SaveParam (ESAVEPARIX ix, TSPDATA src_dat, bool f_phisical_save);
		static void GetParam (ESAVEPARIX ix, TSPDATA dest_dat);
	
		static bool Save ();
		static bool Load ();
	
		static bool BadCRC_check ();
	
		static unsigned char GetNodeID ();
		static unsigned long GetRawParam (ESAVEPARIX ix);
	
};




#endif

