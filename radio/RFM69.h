#ifndef _h_RFM69_h_
#define _h_RFM69_h_

#include "RFM69registers.h"
#include "stdint.h"
#include "SYSBIOS.H"
#include "spi_class.h"
#include "TFTASKIF.h"
#include "rfbasecmd.h"
#include "hard_rut.h"
#include "rfcmddefine.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;

enum ERFMPINS {ERFMPINS_RESET = 0, ERFMPINS_ISR = 1, ERFMPINS_ENDENUM = 2};


typedef struct {
	void *lRam;
	unsigned short sizes;
} LPBUFR;



extern unsigned short GetBeaconTime ();	// время в фиксированных специфических единицах от начала 0 слота (периодически синхронизируется при приеме пакета)

#define RF69_MAX_DATA_LEN       61  // 61  to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
#define CSMA_LIMIT             -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
//#define RF69_MODE_SLEEP         0   // XTAL OFF
//#define RF69_MODE_STANDBY       1   // XTAL ON
//#define RF69_MODE_SYNTH         2   // PLL ON
//#define RF69_MODE_RX            3   // RX MODE
//#define RF69_MODE_TX            4   // TX MODE
#define null                  0
#define COURSE_TEMP_COEF    -90     // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR 255
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000		// 10000 было
#define RF69_FSTEP    61.035156     // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet) FXOSC = module crystal oscillator frequency 
// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40


#define C_BUFERSIZE_RX (RF69_MAX_DATA_LEN + 10)
enum EFRFGTYPE {EFRFGTYPE_MIDLE = 0, EFRFGTYPE_LAST = 1, EFRFGTYPE_FIST = 2, EFRFGTYPE_SINGLE = 3, EFRFGTYPE_ENDENUM};
enum ERFSW {ERFSW_NONE = 0, ERFSW_INIT, ERFSW_TX, ERFSW_RX, ERFSW_STANBY, ERFSW_ENDENUM};
#define C_FRAGMENTDATA_SIZE (RF69_MAX_DATA_LEN - 2)
#define C_FRAMECRC_SIZEOF 2

#pragma pack (push,1)
/*
typedef struct {
	uint8_t ts_code;	// код фрагмента, инкрементный код последовательности, флаг необходимости подтверждения по принятию последнего фрейма
										// 7b - fist, 6b - last, 5b - ack, 4-0b seq_n
	uint8_t lsize;		// локальный размер
} S_RFFRGM_HDR_T;

typedef struct {
	S_RFFRGM_HDR_T hdr;
	uint8_t data[C_FRAGMENTDATA_SIZE];	// ...... данные
} S_RFFRAGMENT_TAG;
*/
#pragma pack (pop)

#define C_networkID 0x34
#define C_LEDBLINK_TIME 100


class TRFM69: public TFFC, public TRADIOIF {		// IFCRFTX
	
		ERFMODE rfmode;
		const uint16_t c_trnsm_bufer_sizes;
		uint8_t *txbuffer;
		uint8_t *rxbuffer;
		uint8_t receive_size;
		uint8_t isRFM69HW;                     // if RFM69HW model matches high power enable possible
		uint8_t powerLevel;
		uint8_t promiscuousMode;
	
		void rcCalibration();
		uint8_t readReg(uint8_t addr);
		void writeReg(uint8_t addr, uint8_t val);
		void encrypt(const char* key);
		int16_t ic_readRSSI(uint8_t forceTrigger);

		uint8_t readTemperature(uint8_t calFactor);
		void setHighPowerRegs(uint8_t onOff);
		
		uint8_t ReadFromFifo (uint8_t *dst, uint16_t maxsz);

		ISPI *SPIx;
		bool GetIntRFMLevel (void);
		
		SYSBIOS::Timer Indik_timeout_tx;
		SYSBIOS::Timer Indik_timeout_rx;

		void ResetPinTo (bool vals_set);
		void ResetPulseToPin ();
		
		virtual void Task ();
		bool get_isr_pin ();
		
		
		void receiveBegin ();
		void ic_setMode (ERFMODE mode);
		void setNetwork (uint8_t networkID);
		void Init ();		// uint8_t nodeID, uint8_t networkID
		void sleep();
		void ic_promiscuous (uint8_t onOff);
		void setHighPower (uint8_t onOFF);           // has to be called after initialize() for RFM69HW
		void setPowerLevel (uint8_t level);            // reduce/increase transmit power level
		uint8_t GetVersion ();
		void mode_sw_to_rx ();

		const S_GPIOPIN *pins_ir;
		const uint16_t c_datapayload_size;
		
	protected:
		ERFMODE getMode();
		uint32_t getFrequency();
		void setFrequency (uint32_t freqHz);
		
		virtual bool GetIndicatorRx ();
		virtual bool GetIndicatorTx ();
		
	
		virtual void tx (S_RFMARKTAG_T *src, ERFMODE endsw_to);
		virtual bool is_tx ();
		virtual bool is_rx ();
		virtual bool rx (S_RFMARKTAG_T *dst,  uint16_t max_dstsz0);
		virtual const uint16_t frame_size ();
	
	public:
		TRFM69 (ISPI *s, const S_GPIOPIN *p);

		//static IFCRFTX *create (ISPI *s, const uint16_t szz, const S_GPIOPIN *p);

};



#endif

