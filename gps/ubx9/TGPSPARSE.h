#ifndef _h_ublox_gps_parse_h_
#define _h_ublox_gps_parse_h_



//#include "rutine.h"
//#include "stddef.h"
#include "stdint.h"
//#include "TTFIFO.h"
#include "mes_statistics.h"

#define C_TIME_LED_RTK_STATUS 10000 // 10 сек

const unsigned char CUBXPREX_A = 0xB5;
const unsigned char CUBXPREX_B = 0x62;

#pragma pack(push ,1)
typedef struct {
	unsigned char Preamble_D3;
	unsigned short len_raw;
} HDRTRANPLAYRTCM3;



typedef struct {
	unsigned char Preamble_A;
	unsigned char Preamble_B;
	unsigned char Class;
	unsigned char Id;
	unsigned short Len;
} HDRUBX;



typedef struct {
	unsigned char CK_A;
	unsigned char CK_B;
} TUBXCRC;

#define C_RTCM3_CRC24_BSIZE 3

typedef struct {
    float data;
    bool f_actual;
} S_FLOAT_T;


typedef struct {
    uint32_t data;
    bool f_actual;
} S_UDIG32_T;


typedef struct {
    char data;
    bool f_actual;
} S_DIG8_T;


typedef struct {
    bool f_actual;
	unsigned char Hour;
	unsigned char Minute;
	float seconds;
}UTCTIME;


typedef struct {
    bool f_actual;
	unsigned char Day;
	unsigned char Month;
	unsigned char Year;
}UTDDATE;


typedef struct {
    bool f_actual;
	uint8_t mod;
}S_POSFIX_MODE_T;

#pragma pack(pop)








/*
class TGPSCBIF {
    public:
        virtual void cb_ubx_data (HDRUBX *inp, uint32_t size) = 0;
        virtual void cb_nmea_data (uint8_t *inp, uint32_t size) = 0;
        virtual void cb_rtcm3_data (uint8_t *inp, uint32_t size) = 0;
};
*/




enum ENMEASRCH {ENMEASRCH_A, ENMEASRCH_B};


class TUBXNMRTCMUX {


        char *linebuf;//[C_GPS_LINEBUF_SIZE];
        uint32_t push_ix;
        const uint32_t C_GPS_LINEBUF_SIZE;

        bool push_raw (uint8_t dat);

        bool CalculateUBX_CRC (uint8_t *lSrc, unsigned short sz, TUBXCRC *lDstCrc);
		unsigned long crc24q_hash(unsigned char *data, unsigned short len);
        bool compareLong24AndBuf (unsigned long curdata, unsigned char *lTxt);
        char *FindFistDelimitter (char *lInpInBuf, uint32_t size, char delimm);
        
    protected:
        // проверяются форматы протоколов в линейном буфере,
        // в случае успеха возвращается размер сообщения и индекс
        // памяти с которого начинается проверенное сообшение
        bool check_nmea_linedata (uint32_t &ix_start, uint32_t &mess_size);
        bool check_ubx_linedata (uint32_t &ix_start, uint32_t &mess_size);
        bool check_rtcm_linedata (uint32_t &ix_start, uint32_t &mess_size);

        uint32_t locfpscnt_nmea;
        uint32_t locfpscnt_ubx;
        uint32_t locfpscnt_rtcm3;

        float rsltfps_nmea;
        float rsltfps_ubx;
        float rsltfps_rtcm3;

        uint32_t locfpscnt_lasttick;

        uint32_t cur_peack_rx_buf;
        TGPSSTATS *statistics;

	public:
		TUBXNMRTCMUX (uint16_t rxbufsize, TGPSSTATS *statistics);
		void In_raw (uint8_t *data, uint16_t sizes);
        virtual void Task ();

        // диагностические функции
        float get_nmea_fps ();
        float get_ubx_fps ();
        float get_rtcm3_fps ();
        void clear_peack_buf_statistics ();
        uint32_t get_peack_buf_size ();


        virtual void cb_ubx_data (HDRUBX *inp, uint32_t size);
        virtual void cb_nmea_data (uint8_t *inp, uint32_t size);
        virtual void cb_rtcm3_data (uint8_t *inp, uint32_t size);

};





class TGPSPCOMN: public TUBXNMRTCMUX {
    protected:
        const static uint8_t C_NMEASIZELONG_DEG;
        const static uint8_t C_NMEASIZELAT_DEG;
        const static uint32_t tmpMultData[10];
        static S_FLOAT_T lon_static;
        static S_FLOAT_T lat_static;
        static S_FLOAT_T alt_static;
        static UTCTIME utc_static;
        static UTDDATE utd_static;
        static S_UDIG32_T satel_cnt_static_all;
        static S_UDIG32_T satel_cnt_static_active;
        static S_POSFIX_MODE_T posfixmod;
        static S_DIG8_T lon_EW;
        static S_DIG8_T lat_NS;
        static S_DIG8_T magnet_EW;
        static S_FLOAT_T magnet_var;
        static S_FLOAT_T vdop;
        static S_FLOAT_T hdop;
        static S_FLOAT_T pdop;

        static uint32_t Timer_RTK_implement;

        static uint32_t CheckDecimal (char *lTxt, uint32_t sz);
        static bool TxtToULong (char *lpRamData, unsigned char sz, uint32_t *lpDataOut);

        virtual void cb_ubx_data (HDRUBX *inp, uint32_t size);
        virtual void cb_nmea_data (uint8_t *inp, uint32_t size);
        virtual void cb_rtcm3_data (uint8_t *inp, uint32_t size);

        bool ParseFieldUTC (char *lTxt, uint32_t sizes);
        bool ParseFieldNmbsSatelite (char *lTxt, uint32_t sizes);
        bool ParseFieldPosMode (char *lTxt, uint32_t sizes);
        bool ParseFieldLongEW (char *lTxt, uint32_t sizes);
        bool ParseFieldLatitude (char *lTxt, uint32_t sizes);
        bool ParseFieldLongitude (char *lTxt, uint32_t sizes);
        bool ParseFieldLatNS (char *lTxt, uint32_t sizes);
        bool ParseFieldUTD (char *lTxt, uint32_t sizes);
        bool ParseFieldMagnetEW (char *lTxt, uint32_t sizes);
        bool ParseFieldMagnetVar (char *lTxt, uint32_t sizes);
        bool ParseFieldPDOP (char *lTxt, uint32_t sizes);
        bool ParseFieldVDOP(char *lTxt, uint32_t sizes);
        bool ParseFieldHDOP (char *lTxt, uint32_t sizes);
        bool ParseFieldSatActive(char *lTxt, uint32_t sizes);

        bool TxtToFloat (float *lpDest, char *lpTxtIn, uint32_t Sizes);
        uint32_t GetSizeToDelim (char *lTxt, uint32_t sz, char delim);
        uint32_t str_len (char *lsrt);


    public:
        TGPSPCOMN (uint16_t rxbufsize, TGPSSTATS *gpsstat);
        static bool get_latitude (float &val);
        static bool get_longitude (float &val);
        static bool get_altitude (float &val);

        static bool get_utc_hour (uint32_t &val);
        static bool get_utc_minute (uint32_t &val);
        static bool get_utc_second (float &val);

        static bool get_utd_day (uint32_t &val);
        static bool get_utd_month (uint32_t &val);
        static bool get_utd_year (uint32_t &val);

        static bool get_satel_cnt_all (uint32_t &val);
        static bool get_satel_cnt_active (uint32_t &val);


};




#endif

