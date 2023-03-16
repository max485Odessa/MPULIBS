#ifndef _H_GPS_STM32_NMEA_USART_H_
#define _H_GPS_STM32_NMEA_USART_H_



#include "resources.h"
#include "TUSARTIFACE.h"
#include "stm32l4xx_hal.h"
#include "SYSBIOS.H"
#include "rutine.h"
#include "fix.h"
#include "auxiliary.h"
#include "app.h"
#include "TFTASKIF.h"
	

#define LO(x)	(unsigned char)((x) & 0xff)
#define MID(x)	(unsigned char)(((x) >> 8) & 0xff)
#define HI(x)	(unsigned char)(((x) >> 16) & 0xff)


//  46.465085  30.701848
#define WGS84_A 6378137.0
/** Inverse flattening of the Earth, \f$ 1/f \f$.
 * This is a defining parameter of the WGS84 ellipsoid. */
#define WGS84_IF 298.257223563
/** The flattening of the Earth, \f$ f \f$. */
#define WGS84_F (1 / WGS84_IF)
/** Semi-minor axis of the Earth in meters, \f$ b = a(1-f) \f$. */
#define WGS84_B (WGS84_A * (1 - WGS84_F))
/** Eccentricity of the Earth, \f$ e \f$ where \f$ e^2 = 2f - f^2 \f$ */
#define WGS84_E (sqrt(2 * WGS84_F - WGS84_F * WGS84_F))


#ifndef M_PI
#define M_PI 3.14159265358979323846264338
#endif

#ifndef M_PI_2
#define M_PI_2 1.5707963267948966192313216
#endif

#define R2D (180.0 / M_PI) /**< Conversion factor from radians to degrees. */
#define D2R (M_PI / 180.0) /**< Conversion factor from degrees to radians. */
#define AS2R (D2R / 3600.0)

/*
ZED-F9P

GNRMC
GNVTG

GNGGA
GNGSA
GNGSA
GNGSA
GNGSA

GPGSV
GPGSV
GPGSV
GPGSV
GPGSV
GPGSV
GPGSV
GPGSV

GLGSV
GLGSV
GLGSV
GLGSV

GAGSV
GAGSV

GBGSV
GAGSV

GNGLL
*/



#define C_PERIODIC_CBTIME 1
//#define C_USART_GPS_PORT USART1		// сменить порт GPS тут, но это не исключит переинициализацию GPIO в Init методе
#define C_GPS_TIMEOUT_FRAME 3			// 5 ms таймаут синхронизации GPS приема (покой на линии)
#define C_GPSRECVBUF_SIZE 2048		// размер буфера под полный пакет данных, меняем под нужный GPS для оптимизации оперативной памяти
#define C_DOP_NAN 100.0


const unsigned char C_NMEASIZELONG_DEG = 3;
const unsigned char C_NMEASIZELAT_DEG = 2;

const double C_MERIDIAN_360C_KM = 40008.55;						// км
const double C_MERIDIAN_90C_KM = C_MERIDIAN_360C_KM / 4;	// км
const double C_MERIDIAN_1C_KM = C_MERIDIAN_90C_KM / 90;		// км
const double C_MERIDIAN_1MIN_KM = C_MERIDIAN_1C_KM / 60;	// сколько километров в минуте
const double C_MERIDIAN_1SEK_KM = C_MERIDIAN_1MIN_KM / 60;	// сколько километров в секунде


extern const char *ltxt_gngga, *ltxt_gpgga, *ltxt_gnrmc, *ltxt_gprmc, *ltxt_gngsa, *ltxt_gpgsa, *ltxt_gngll, *ltxt_gpgll;


enum EGNSSINX {EGNSSINX_UTC = 1, EGNSSINX_LAT = 2, EGNSSINX_LAT_NS = 3, EGNSSINX_LONG = 4, EGNSSINX_LON_EW = 5, EGNSSINX_QUAL = 6, EGNSSINX_NBRSSATEL = 7, EGNSSINX_HDOP = 8, EGNSSINX_ALTIT = 9, EGNSSINX_D_ELIPS = 10};
const char C_GNSS_MAXFIELD = 14;

enum EGGAINX {EGGAINDX_UTC = 1, EGGAINDX_LAT = 2, EGGAINDX_LAT_NS = 3, EGGAINDX_LONG = 4, EGGAINDX_LON_EW = 5, EGGAINDX_QUAL = 6, EGGAINDX_NBRSSATEL = 7, EGGAINDX_HDOP = 8, EGGAINDX_ALTIT = 9, EGGAINDX_D_ELIPS = 11};
const char C_GGA_MAXFIELD = 14;

enum ERMCINX {ERMCINDX_UTC = 1, ERMCINDX_VALID = 2, ERMCINDX_LAT = 3, ERMCINDX_LAT_NS = 4, ERMCINDX_LONG = 5, ERMCINDX_LON_EW = 6, ERMCINDX_SPEEDOV = 7, ERMCINDX_COURSE = 8, ERMCINDX_UTD = 9, ERMCINDX_MAGNETVAR = 10, ERMCINDX_MAGEW = 11};
const char C_RMC_MAXFIELD = 11;
	
const char C_GSA_MAXFIELD = 17;
enum EGSAINX {EGSAINX_FIXMOD = 2, EGSAINX_PDOP = 15, EGSAINX_HDOP = 16, EGSAINX_VDOP = 17};

const char C_GSV_MAXFIELD = 18;
enum EGSVINX {EGSVINX_VISIBSAT = 3};


typedef struct {
	unsigned char Hour;
	unsigned char Minute;
	//unsigned char Second;
	float seconds;
}UTCTIME;


typedef struct {
	unsigned char Day;
	unsigned char Month;
	unsigned char Year;
}UTDDATE;



typedef struct {
	UTCTIME UTC_time;
	UTDDATE UTD_date;
	//uavcan_equipment_gnss_Fix Fix2;
	
	TTDOPTAG PDOP;
	TTDOPTAG VDOP;
	TTDOPTAG HDOP;
	TTDOPTAG EDOP;
	TTDOPTAG NDOP;
	TTDOPTAG TDOP;
	TTDOPTAG GDOP;
	TTVELOCITYTAG VELN;
	TTVELOCITYTAG VELE;
	TTVELOCITYTAG VELD;
	
	TCOVRPARAMS COVARIANCES;
	
	//TTVALTAG PSCOVR_0;
	//TTVALTAG PSCOVR_1;
	//TTVALTAG PSCOVR_2;
	//TTVALTAG VELCOVR;
	unsigned long height_ellipsoid_mm;
	unsigned long height_msl_mm;

	
	float MagVariation;
	float TrueCourse;
	float Speed;
	double Latitude;
	double Longitute;
	float NmeaAltitude;
	float NmeaDifElipsHeight;
	char Lat_NS;
	char Lon_EW;
	char Mag_EW;
	//char FixQuality;
	char ValidityStatus;
	char Mode2D3D;
	char SatelitAmount;
	char SatelitViewN;
	
	bool f_UTC_valid;	
	bool f_UTD_valid;	
	bool f_Lat_valid;
	bool f_Lat_NS_valid;
	bool f_Long_valid;
	bool f_Long_EW_valid;
	bool f_Nmea_Altitude_valid;
	bool f_Course_valid;
	bool f_Speed_valid;
	bool f_SatelitNumb_valid;
	bool f_SatViewN_valid;
	bool f_MagnetVAR_valid;
	bool f_MagnetVAR_EW_valid;
	//bool f_Quality_valid;
	bool f_ValidField_valid;
	bool f_FixMode_valid;
	bool f_height_elips_valid;
	bool f_height_msl_valid;

	bool f_Nmea_DifElipsHeight_valid;
	
} GPSFULLDATA;







typedef __packed struct {
	unsigned char CK_A;
	unsigned char CK_B;
} TUBXCRC;



typedef __packed struct {
	unsigned char Preamble_A;
	unsigned char Preamble_B;
	unsigned char Class;
	unsigned char Id;
	unsigned short Len;
} HDRUBX;








typedef __packed struct {
	HDRUBX Hdr;
	unsigned char portID;
	unsigned char reserved1;
	unsigned short txReady;
	unsigned long mode;
	unsigned long baudRate;
	unsigned short inProtoMask;
	unsigned short outProtoMask;
	unsigned short flags;
	unsigned char reserved2[2];
} UBXSETSPEED;


typedef __packed struct {
	unsigned long key_id;
	unsigned char value;
} IDCFGKEY1;


typedef __packed struct {
	unsigned long key_id;
	unsigned short value;
} IDCFGKEY2;



typedef __packed struct {
	unsigned long key_id;
	unsigned long value;
} IDCFGKEY4;



typedef __packed struct {
	HDRUBX Hdr;
	unsigned char version;
	unsigned char powerSetupValue;
	unsigned short period;
	unsigned short onTime;
	unsigned char reserved1[2];
} UBXSETPMS;



typedef __packed struct {
	HDRUBX Hdr;
	unsigned short measRate;
	unsigned short navRate;
	unsigned short timeRef;
} UBXSETRATE;



typedef __packed struct {
	HDRUBX Hdr;
	unsigned char dgnssMode;
	unsigned char reserved1[3];
} UBXCFGDGNSS;



typedef __packed struct {
	HDRUBX Hdr;
	unsigned char version;
	unsigned char layers;
	unsigned char reserved[2];
} UBXCGFVERS0;




#define C_ROVERMESSAGE_AMOUNT 6





typedef __packed struct {
	unsigned long iTOW;
	unsigned short gDOP;
	unsigned short pDOP;
	unsigned short tDOP;
	unsigned short vDOP;
	unsigned short hDOP;
	unsigned short nDOP;
	unsigned short eDOP;
} FUBXDOP;






typedef __packed struct {
	unsigned long iTOW;					// GPS time of week
	unsigned short year;				// Year (UTC)
	unsigned char month;				// Month, range 1..12 (UTC)
	unsigned char day;					// Day of month, range 1..31 (UTC)
	unsigned char hour;					// Hour of day, range 0..23 (UTC)
	unsigned char min;					// Minute of hour, range 0..59 (UTC)
	unsigned char sec;					// Seconds of minute, range 0..60 (UTC)
	unsigned char valid;				// Validity flags
	unsigned long tAcc;					// Time accuracy estimate (UTC)
	long nano;									// Fraction of second, range -1e9 .. 1e9 (UTC)
	unsigned char fixType;			// 0: no fix, 1: dead reckoning only, 2: 2D-fix, 3: 3D-fix, 4: GNSS + dead reckoning combined, 5: time only fix
	unsigned char flags;				// Fix status flags
	unsigned char flags2;				// Additional flags
	unsigned char numSV;				// Number of satellites used in Nav Solution
	long lon;										// Longitude 1e-7
	long lat;										// Latitude 1e-7
	long height;								// Height above ellipsoid
	long hMSL;									// Height above mean sea level
	unsigned long hAcc;					// Horizontal accuracy estimate
	unsigned long vAcc;					// Vertical accuracy estimate
	long velN;									// NED north velocity
	long velE;									// NED east velocity
	long velD;									// NED down velocity
	long gSpeed;								// Ground Speed (2-D)
	long headMot;								// Heading of motion (2-D) 1e-5
	unsigned long sAcc;					// Speed accuracy estimate
	unsigned long headAcc;			// Heading accuracy estimate (both motion and vehicle) 1e-5
	unsigned short pDOP;				// Position DOP	0.01
	unsigned char flags3;				// valid lat, lon, height
	unsigned char reserved[5];
	long headVeh;								// Heading of vehicle (2-D)	1e-5
	short magDec;								// Magnetic declination	1e-2
	unsigned short magAcc;			// Magnetic declination accuracy	1e-2
} FUBXPVT;



typedef __packed struct {
	unsigned char version;					
	unsigned char reserved1;
	unsigned short refStationId;
	unsigned long iTOW;
	long relPosN;					
	long relPosE;
	long relPosD;
	long relPosLength;
	long relPosHeading;
	unsigned char reserved2[4];
	char relPosHPN;
	char relPosHPE;
	char relPosHPD;
	char relPosHPLength;
	unsigned long accN;
	unsigned long accE;
	unsigned long accD;
	unsigned long accLength;
	unsigned long accHeading;
	unsigned char reserved3[4];
	unsigned long flags;
} FUBXRELPOSNED;




typedef __packed struct {
	unsigned char clsID;
	unsigned char msgID;
} FUBXNACK;



const unsigned long C_CFG_NAVHPG_DGNSSMODE = 0x20140011;							// E1 2 or 3





const unsigned long C_CFG_MSGOUT_UBX_NAV_SIG_UART1 = 0x20910346;				// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_POSLLH_UART1 = 0x2091002a;			// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_RELPOSNED_UART1 = 0x2091008e;	// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_SVIN_UART1 = 0x20910089;				// u1


const unsigned long C_CFG_MSGOUT_UBX_NAV_PVT_UART1 = 0x20910007;				// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_SAT_UART1 = 0x20910016;				// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_STATUS_UART1 = 0x2091001b;			// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_DOP_UART1 = 0x20910039;				// u1
const unsigned long C_CFG_MSGOUT_UBX_NAV_VELNED_UART1 = 0x20910043;				// u1
const unsigned long C_CFG_MSGOUT_UBX_MON_RF_UART1 = 0x2091035a;				// u1

const unsigned long CFG_MSGOUT_NMEA_ID_GNS_UART1 = 0x209100b6;	// u1


const unsigned long C_CFG_RATE_MEAS = 0x30210001;			// u2  0.001s



const unsigned long C_CFG_TMODE_MODE = 0x20030001;					// E1   0=DISABL, 1=Survey In, 2=FIX 
const unsigned long C_CFG_TMODE_POS_TYPE = 0x20030002;			// E1   1=(lat,lon,height)
const unsigned long C_CFG_TMODE_ECEF_X = 0x40030003;				// i4   cm
const unsigned long C_CFG_TMODE_ECEF_Y = 0x40030004;				// i4   cm
const unsigned long C_CFG_TMODE_ECEF_Z = 0x40030005;				// i4 	cm
const unsigned long C_CFG_TMODE_LAT = 0x40030009;						// i4		1e-7 deg
const unsigned long C_CFG_TMODE_LON = 0x4003000a;						// i4		1e-7 deg
const unsigned long C_CFG_TMODE_HEIGHT = 0x4003000b;				// i4		cm
const unsigned long C_CFG_TMODE_ECEF_X_HP = 0x20030006;			// i1   0.1 mm
const unsigned long C_CFG_TMODE_ECEF_Y_HP = 0x20030007;			// i1		0.1 mm
const unsigned long C_CFG_TMODE_ECEF_Z_HP = 0x20030008;			// i1		0.1 mm
const unsigned long C_CFG_TMODE_LAT_HP = 0x2003000c; 				// i1		1e-9 deg
const unsigned long C_CFG_TMODE_LON_HP = 0x2003000d;				// i1		1e-9 deg
const unsigned long C_CFG_TMODE_HEIGHT_HP = 0x2003000e;			// i1   0.1 mm
const unsigned long C_CFG_TMODE_FIXED_POS_ACC = 0x4003000f;	// u4		0.1



const unsigned long RTKKEYARRAY_QGROUNDUBX[7] = {C_CFG_MSGOUT_UBX_NAV_SVIN_UART1, C_CFG_MSGOUT_UBX_NAV_PVT_UART1, C_CFG_MSGOUT_UBX_NAV_SAT_UART1, C_CFG_MSGOUT_UBX_NAV_STATUS_UART1, C_CFG_MSGOUT_UBX_NAV_DOP_UART1, C_CFG_MSGOUT_UBX_NAV_VELNED_UART1, C_CFG_MSGOUT_UBX_MON_RF_UART1};
const unsigned long RTKROVERENABLEARRAY[C_ROVERMESSAGE_AMOUNT] = {C_CFG_MSGOUT_UBX_NAV_PVT_UART1, C_CFG_MSGOUT_UBX_NAV_POSLLH_UART1, C_CFG_MSGOUT_UBX_NAV_RELPOSNED_UART1, C_CFG_MSGOUT_UBX_NAV_STATUS_UART1, C_CFG_MSGOUT_UBX_NAV_SVIN_UART1, CFG_MSGOUT_NMEA_ID_GNS_UART1};



typedef __packed struct {
	UBXCGFVERS0 Base;

	IDCFGKEY4 lat;
	IDCFGKEY4 lon;
	IDCFGKEY4 height;

	IDCFGKEY1 pos_type;
	IDCFGKEY1 mode;
	IDCFGKEY2 rate_meas;	
	
	IDCFGKEY4 ecef_x;
	IDCFGKEY4 ecef_y;
	IDCFGKEY4 ecef_z;
	IDCFGKEY1 ecef_x_hp;
	IDCFGKEY1 ecef_y_hp;
	IDCFGKEY1 ecef_z_hp;

} UBXCGFRTKSTATION;



typedef struct {
	double x;
	double y;
	double z;
} ECEF_POINT;



typedef struct {
	double latitude;
	double longitude;
	double altitude;
} WGS84POINT;



typedef __packed struct {
	UBXCGFVERS0 Base;
	IDCFGKEY2 rate_meas;
	IDCFGKEY1 rover_mess[C_ROVERMESSAGE_AMOUNT];

} UBXCGFRTKROVER;


enum EUBXCLASS {EUBXCLASS_NAV = 1, EUBXCLASS_ACK = 5, EUBXCLASS_CFG = 6, EUBXCLASS_MON = 0x0A};
enum EUBXACK {EUBXACK_NACK = 0, EUBXACK_ACK = 1};
enum EUBXMON {EUBXMON_RF = 0x38};
enum EUBXNAV {EUBXNAV_POSLLH = 0x02, EUBXNAV_STATUS = 0x03, EUBXNAV_DOP = 0x04, EUBXNAV_PVT = 0x07, EUBXNAV_VELNED = 0x12, EUBXNAV_SAT = 0x35, EUBXNAV_SVIN = 0x3B, EUBXNAV_RELPOSNED = 0x3C};
enum EUBXCFG {EUBXCFG_PRT = 0, EUBXCFG_RATE = 8, EUBXCFG_DGNSS = 0x70, EUBXCFG_PMS = 0x86, EUBXCFG_VALSET = 0x8A, EUBXCFG_VALGET = 0x8B, EUBXCFG_VALDEL = 0x8D};
enum EUBXMEMTYPE {EUBX_RAM_LAY = 0, EUBX_BBR_LAY = 1, EUBX_FLASH_LAY = 2, EUBX_DEF_LAY = 7};
enum EGPSMESSTYPE {EGPSMESSTYPE_NONE = 0, EGPSMESSTYPE_UBX = 1, EGPSMESSTYPE_NMEA = 2};
enum EGPSSW {EGPSSW_SPEED_DETECT_A = 0, EGPSSW_WORK = 1};
enum EUSARTSPD {EUSARTSPD_9600 = 0, EUSARTSPD_38400 = 1, EUSARTSPD_115200 = 2, EUSARTSPD_230400 = 3, EUSARTSPD_460800 = 4, EUSARTSPD_MAX = 5};
enum ERTKSUBMODE {ERTKSUBMODE_FLOAT = 2, ERTKSUBMODE_FIXED = 3};


const unsigned long C_NEED_WORK_SPEED = 460800;
const unsigned long C_USARTSPEED_INDX[EUSARTSPD_MAX] = {9600,38400,115200,230400,C_NEED_WORK_SPEED};
const unsigned long C_SPEEDSET_TIMEOUT = 1500;			// полторы секунды поиск любых пакетов с нормальной CRC на текущей скорости
const unsigned long C_GPS_NODATA_TIMEOUT = 3000;		// если с GPS в течении трех секунд нет данных с нормальной CRC - переходим к детектированию скорости


// EGPSSW_SPEED_DETECT_A - определение текущей скорости
// 				(переключается скороть USART, определенный таймаут прослушивается линия, если нет данных скорость меняется по кругу из стандартных скоростей)
// EGPSSW_SET_SPEED - установка нужной скорости
// 				(когда текущая скорость известна, устанавливается необходимая (115200))
// EGPSSW_SPEED_DETECT_B - определение что нужная скорость установилась
//				(детектируется скорость 115200 и если скорость установилась переходим к EGPSSW_WORK) 
// EGPSSW_WORK - обычная работа


//enum EROVCORSTEP {EROVCORSTEP_CHECKINPUT = 0, EROVCORSTEP_NEEDTX = 1};
enum EUBXREQ {ENEXTREQ_DOP = 0, ENEXTREQ_PVT = 1, ENEXTREQ_RATE = 2, ENEXTREQ_END = 3};


const unsigned char CUBXPREX_A = 0xB5;
const unsigned char CUBXPREX_B = 0x62;
const float C_DOP_NANVALUE = 100.0;
const unsigned char C_MATTX_TIMEOUT = 100;
const utimer_t C_PERIOD_DOP = 2500;		

const utimer_t C_PERIOD_RATE = 180000;	
const utimer_t C_PERIOD_RTK = 181000;


#define C_TIME_LED_RTK_STATUS 3000


class TCANARDGPS: public TUSART_IFACE, public TFFC {
	private:
		
		utimer_t DopsTimer;
		utimer_t ErrorTimeout;
		// Таймеры периодических отправок
		utimer_t UBXReqTimet_DOP;
		utimer_t UBXReqTimet_PVT;
		utimer_t UBXReqTimet_RATE;
		//static utimer_t Timer_RTK_implement;
	
		unsigned long C_PERIOD_PVT;
	
		EUBXREQ swUBXreq;
	
		/*
		// субрутины для вычисления  контрольной суммы rtcm
		static unsigned long crc24q_hash(unsigned char *data, unsigned short len);
		static bool compareLong24AndBuf (unsigned long curdata, unsigned char *lTxt);
		static void crc24q_sign(unsigned char *data, short len);
		static bool crc24q_check(unsigned char *data, short len);
		*/
	
		GPSFULLDATA GPSAccData;													// структура в которой содержатся все текущие параметры принятые с gps сообщений (NMEA и UBX)
		unsigned char buf_tx[1280];											// буфер передачи
		static char bufer_p[C_GPSRECVBUF_SIZE];					// буфер приема
		unsigned long b_indx;														// индексный указатель на буфер приема
		virtual void ISR_DataRx (unsigned char dat);
	
	
		bool check_recive_data ();															// проверяет f_is_nmea флаг
		bool f_is_recived_data;																	// есть сырые принятые данные от GPS (и буфер заблокирован от поступления данных)
		unsigned long nmea_size;												// размер принятых данных с GPS
							
		bool f_new_gps_data;														// флаг что есть отпарсенные данные

		static void LowHard_CB (TCANARDGPS *lThis);			// технический CallBBack
		void PeriodicCall_1Ms ();												// интервальный CallBack для таймаута (по синхронизации данных)
		unsigned long LastdataTimeout;									// отслеживание интервала покоя на линии
	
		bool f_sync_success;														// синхронизация проведена 
		void NeedSyncFrame ();													// перезапуск синхронизации
		void ClearBufer ();				
		void DataInBuferStatus ();
		
		
		// субрутины по UBX пакетам
		unsigned short GetUBXFrameSize (HDRUBX *lframe);	// просчитывает текущий размер UBX пакета (header + data)
		void WriteUBXSignature (HDRUBX *ldst);					
		void WriteUBX_CRC (HDRUBX *ldst);
		bool CalculateUBX_CRC (unsigned char *lSrc, unsigned short sz, TUBXCRC *lDstCrc);
		
		bool CheckUBX_Format (HDRUBX *lframe, unsigned long *cut_sz);											// проверяет входной буфер на соответствие формату UBX и возвращает размер записи
		bool CheckNMEA_Format (char *lTxt, unsigned long *cut_sz);													// проверяет входной буфер на соответствие формату NMEA и возвращает размер записи

		
		void CheckUpdateDOP (unsigned short inp_val, TTDOPTAG &dopval);
		void CheckUpdateVelocity (long inp_val, TTVELOCITYTAG &dopval);
		
		bool ParseUBX_OneLine (char *lpLine, unsigned long &d_size);											// парсит одно UBX сообщение, возвращает размер линии
		EGPSMESSTYPE CheckLineType (char *lpLine);																				// возвращает формат сообщения UBX или NMEA
		
		bool ParseNMEA_OneLine (char *lpLine, unsigned long &d_size);											// парсит одно NMEA сообщение, возвращает размер линии	
		
		EGPSSW SWVal;
		
		void NextSpeedSearch ();
		void UBX_SpeedSet_460800 ();
		void ToDetectUsartSpeed ();
		EUSARTSPD speed_search;
		
	protected:
		char *FindFistDelimitter (char *lInpInBuf, char delimm);
	
		unsigned long GetLenGPS_NMEATag (char *lsrc);
		char *GetFistStringTag (unsigned long *cut_sz, EGPSMESSTYPE &typetag);
		char *GetNextStringTag (unsigned long *cut_sz, EGPSMESSTYPE &typetag);
		char *lCurFindAdr;
		unsigned long CurFindTagSize;
	
		void SendUBX_NAV_DOP_req ();
		void SendUBX_NAV_PVT_req ();
		void SendUBX_CFG_RATE_req (unsigned short rate_ms);
		void SendUBX_SetSpeed (unsigned long spdset);
		void SendUBX_SetPMS ();
		void SendUBX_SetDGNSS (ERTKSUBMODE modd);
		void SendUBX_CFGVAL_set (void *InArray, unsigned long sizes, EUBXMEMTYPE lay);
		void SendUBX_CFGVAL_get (void *InArray, unsigned long sizes, EUBXMEMTYPE lay);
		void SendUBX_CFGVAL_del (void *InArray, unsigned long sizes, EUBXMEMTYPE lay);


	
		unsigned char ParseNMEA_Frames ();
		// извлечение информации из NMEA сообщений
		bool ParseGPGNSLine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseGNSSLine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseGGALine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseRMCLine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseGSALine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseGSVLine (char *lTxt, unsigned long size, unsigned long amount_filed);
		bool ParseFieldUTC (char *lTxt, unsigned long sizes);
		bool ParseFieldLatitude (char *lTxt, unsigned long sizes);
		bool ParseFieldLatNS (char *lTxt, unsigned long sizes);
		bool ParseFieldLongitude (char *lTxt, unsigned long sizes);
		bool ParseFieldLongEW (char *lTxt, unsigned long sizes);
		//bool ParseFieldQuality (char *lTxt, unsigned long sizes);
		bool ParseFieldPosMode (char *lTxt, unsigned long sizes);
		bool ParseFieldNmbsSatelite (char *lTxt, unsigned long sizes);
		bool ParseFieldAltitude (char *lTxt, unsigned long sizes);
		bool ParseFieldValid (char *lTxt, unsigned long sizes);
		bool ParseFieldSpeedOV (char *lTxt, unsigned long sizes);
		bool ParseFieldCourse (char *lTxt, unsigned long sizes);
		bool ParseFieldUTD (char *lTxt, unsigned long sizes);
		bool ParseFieldMagnetVar (char *lTxt, unsigned long sizes);
		bool ParseFieldMagnetEW (char *lTxt, unsigned long sizes);
		bool ParseFieldPDOP(char *lTxt, unsigned long sizes);
		bool ParseFieldVDOP(char *lTxt, unsigned long sizes);
		bool ParseFieldHDOP (char *lTxt, unsigned long sizes);
		//bool ParseFieldFixMode(char *lTxt, unsigned long sizes);
		bool ParseFieldDifElipsHeight (char *lTxt, unsigned long sizes);
		bool ParseFieldSatView(char *lTxt, unsigned long sizes);
		
		// несколько функций по извлечению информации из UBX пакетов
		bool ParseUBXLine_DOP (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_PVT (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_NACK (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_ACK (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_NAVPOSLLH (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_NAVRELPOSNED (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_NAVSTATUS (char *lTxt, unsigned long sizes);
		bool ParseUBXLine_NAVSVIN (char *lTxt, unsigned long sizes);
		

		unsigned short Parse ();

		
		static uint8_t cur_sat_amount;
		static uint8_t gpsDmode;
	
	public:
		TCANARDGPS ();
	
		void Init (unsigned long speed_set);			
		virtual void Task ();
	
		bool CheckNewData ();																			// проверить есть ли данные
		bool GetData (uavcan_equipment_gnss_Fix &datt);						// получить распарсенные данные
		bool GetAuxData (uavcan_equipment_gnss_Auxiliary &datt);
		TCOVRPARAMS *GetCovariances ();
		void ClearFlagNewData ();
		void SetRate_PVT (unsigned long v_pvt);
		
		static uint8_t GetCurentSat_count ();
		static uint8_t GetDimensModeGPS ();
	
	
};





#endif

