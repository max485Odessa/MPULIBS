#ifndef _H_SI4431_SILICON_LABS_RF_MODULE_H_
#define _H_SI4431_SILICON_LABS_RF_MODULE_H_


#include <stdint.h>
#include "TTFIFO.h"
#include "TFTASKIF.h"
#include "SYSBIOS.H"
#include "spi_class.h"


enum EDATRATE {EDATRATE_7200 = 0, EDATRATE_9600, EDATRATE_12000, EDATRATE_14400, EDATRATE_16800, EDATRATE_19200, EDATRATE_21600, EDATRATE_24000, EDATRATE_ENDENUM = 8};

#define C_RAWFRAME_SIZE 64
#define C_RF_PREAMBLE_SIZE 0x0A
#define C_RF_SYNC_LEN 2
#define C_RF_CRC_SIZE 2



// register addresses
#define RFM22B_INTSTAT1     0x03
#define RFM22B_INTSTAT2     0x04
#define RFM22B_INTEN1        0x05
#define RFM22B_INTEN2        0x06
#define RFM22B_OPMODE1     0x07
#define RFM22B_OPMODE2     0x08
#define RFM22B_XTALCAP      0x09
#define RFM22B_MCUCLK       0x0A
#define RFM22B_GPIOCFG0    0x0B
#define RFM22B_GPIOCFG1    0x0C
#define RFM22B_GPIOCFG2    0x0D
#define RFM22B_IOPRTCFG    0x0E

#define RFM22B_IFBW           0x1C
#define RFM22B_AFCLPGR      0x1D
#define RFM22B_AFCTIMG      0x1E
#define RFM22B_RXOSR         0x20
#define RFM22B_NCOFF2       0x21
#define RFM22B_NCOFF1       0x22
#define RFM22B_NCOFF0       0x23
#define RFM22B_CRGAIN1     0x24
#define RFM22B_CRGAIN0     0x25
#define RFM22B_RSSI           0x26
#define RFM22B_AFCLIM       0x2A
#define RFM22B_AFC0          0x2B
#define RFM22B_AFC1          0x2C

#define RFM22B_DACTL    0x30
#define RFM22B_HDRCTL1    0x32
#define RFM22B_HDRCTL2    0x33
#define RFM22B_PREAMLEN  0x34
#define RFM22B_PREATH      0x35
#define RFM22B_SYNC3        0x36
#define RFM22B_SYNC2        0x37
#define RFM22B_SYNC1        0x38
#define RFM22B_SYNC0        0x39

#define RFM22B_TXHDR3      0x3A
#define RFM22B_TXHDR2      0x3B
#define RFM22B_TXHDR1      0x3C
#define RFM22B_TXHDR0      0x3D
#define RFM22B_PKTLEN       0x3E
#define RFM22B_CHKHDR3    0x3F
#define RFM22B_CHKHDR2   0x40
#define RFM22B_CHKHDR1   0x41
#define RFM22B_CHKHDR0   0x42
#define RFM22B_HDREN3     0x43
#define RFM22B_HDREN2     0x44
#define RFM22B_HDREN1     0x45
#define RFM22B_HDREN0     0x46
#define RFM22B_RXPLEN      0x4B

#define RFM22B_TXPOWER   0x6D
#define RFM22B_TXDR1        0x6E
#define RFM22B_TXDR0        0x6F

#define RFM22B_MODCTL1      0x70
#define RFM22B_MODCTL2      0x71
#define RFM22B_FREQDEV      0x72
#define RFM22B_FREQOFF1     0x73
#define RFM22B_FREQOFF2     0x74
#define RFM22B_BANDSEL      0x75
#define RFM22B_CARRFREQ1  0x76
#define RFM22B_CARRFREQ0  0x77
#define RFM22B_FHCH           0x79
#define RFM22B_FHS             0x7A
#define RFM22B_TX_FIFO_CTL1    0x7C
#define RFM22B_TX_FIFO_CTL2    0x7D
#define RFM22B_RX_FIFO_CTL     0x7E
#define RFM22B_FIFO            0x7F

// register fields
#define RFM22B_OPMODE_POWERDOWN    0x00
#define RFM22B_OPMODE_READY    0x01  // enable READY mode
#define RFM22B_OPMODE_TUNE      0x02  // enable TUNE mode
#define RFM22B_OPMODE_RX      0x04  // enable RX mode
#define RFM22B_OPMODE_TX      0x08  // enable TX mode
#define RFM22B_OPMODE_32K      0x10  // enable internal 32k xtal
#define RFM22B_OPMODE_WUT    0x40  // wake up timer
#define RFM22B_OPMODE_LBD     0x80  // low battery detector

#define RFM22B_PACKET_SENT_INTERRUPT               0x04
#define RFM22B_RX_PACKET_RECEIVED_INTERRUPT   0x02



typedef struct {
	uint8_t raw[C_RAWFRAME_SIZE];
} S_RAWRADIO_CAPSUL_T;


typedef struct {
	union {
				uint8_t header[4];
				uint32_t hdr32;
				} hform;
} S_SI4431HDR_T;



enum ERDSTATUS {ERDSTATUS_READ_NONE = 0, ERDSTATUS_READ_OK = 1, ERDSTATUS_READ_ERROR = 2, ERDSTATUS_READ_END = 3};
enum EWRSTATUS {EWRSTATUS_WRITE_OK = 0, EWRSTATUS_WRITE_ERROR = 1, EWRSTATUS_WRITE_END = 2};



class TRADIOIF {
	
	protected:
		EDATRATE cur_datarate;
		uint8_t cur_rf_power;

	public:
		//virtual void SetDeviation (float hz) = 0;
		void preinit_set_baudrate (EDATRATE val);
		void preinit_set_power (uint8_t val);
	
		virtual void FifoReset () = 0;
		virtual void SetFreq (float fr) = 0;
		virtual void SetFreqOffset (long ofs) = 0;
		virtual void SetCannel (uint8_t cnl) = 0;
		//virtual void SetBps (uint32_t val) = 0;
		virtual void SetModemSpeed (EDATRATE val) = 0;
		virtual void Transmit (const S_RAWRADIO_CAPSUL_T &dat, uint8_t len) = 0;
		virtual void HeaderSet (bool val, S_SI4431HDR_T *fltr) = 0;
		virtual bool CheckReceiver () = 0;
		virtual uint8_t Read (S_RAWRADIO_CAPSUL_T *dst) = 0;
		virtual uint8_t Read (uint8_t *dst, uint8_t maxbuf) = 0;
		virtual bool FastScanRSSI (char *dstval) = 0;
		virtual void RxOn (bool val) = 0;
		virtual long GetCannelFreq (uint8_t cnl) = 0;
		virtual ERDSTATUS GetReadStatus () = 0;
		virtual EWRSTATUS GetWriteStatus () = 0;
		virtual bool NIRQ () = 0;
		virtual void rfm23_burst(bool write, uint8_t address, uint8_t *data, uint8_t n) = 0;
		virtual void RxMode () = 0;
		virtual void TxMode () = 0;
		virtual void IdleMode () = 0;
		virtual void radio_init () = 0;
		virtual void SetPower (u8 TXPower) = 0;
		virtual uint8_t GetRSSI () = 0;
		virtual uint8_t GetPGA () = 0;
		virtual uint32_t convert_to_bps (EDATRATE val) = 0;
		
};




class TSI4431Class: public TRADIOIF, public TFFC {
		//TTFIFO<S_RAWRADIO_CAPSUL_T> *fifo_RX;										 // блоки данных в радиоканал
		//TTFIFO<S_RAWRADIO_CAPSUL_T> *fifo_TX;										 // блоки данных из радиоканала в usart
	
		const float c_freq_start_band_Mhz;
		const float c_freq_stop_band_Mhz;
		const float c_freq_chann_width_Mhz;
		const unsigned long c_chann_amount;
	
		virtual void Task ();
	
		TSPI *spi;
	
		float current_freq_val;
		long current_freq_ofs;
	
		S_SI4431HDR_T current_header;
		bool f_header_en;
	
		void set_freq_raw (float val);
		void set_header_raw (S_SI4431HDR_T *val);
		
		void WriteRegister (u8 reg, u8 value);
		u8 ReadRegister (u8 reg);
		void EZ_RSTATUS (u8 &st1, u8 &st2);
		void EZ_SWRST ();
		void EZ_WAIT_NIRQ ();
		
		
		void mux_tx_mode ();
		void mux_rx_mode ();
		void mux_idle_mode ();
		
	public:
		TSI4431Class (const S_SPIPINS_T *spipns, float strt_f_mhz, float stp_f_mhz, float ch_f_mhz);
		virtual void SetFreq (float fr);
		virtual void SetCannel (uint8_t cnl);
		virtual long GetCannelFreq (uint8_t cnl);
		virtual void SetFreqOffset (long ofs);
		//virtual void SetBps (uint32_t val);
		virtual void SetModemSpeed (EDATRATE val);
		//virtual void SetDeviation (float hz);
		virtual bool NIRQ ();
		virtual ERDSTATUS GetReadStatus ();
		virtual EWRSTATUS GetWriteStatus ();
		virtual void FifoReset ();
		
		virtual void Transmit (const S_RAWRADIO_CAPSUL_T &dat, uint8_t len);
		virtual void HeaderSet (bool val, S_SI4431HDR_T *fltr);
		virtual bool CheckReceiver ();
		virtual uint8_t Read (S_RAWRADIO_CAPSUL_T *dst);
		virtual uint8_t Read (uint8_t *dst, uint8_t maxbuf);
		virtual void rfm23_burst(bool write, uint8_t address, uint8_t *data, uint8_t n);
		virtual bool FastScanRSSI (char *dstval);
	
		virtual void RxOn (bool val);
		virtual void RxMode ();
		virtual void TxMode ();
		virtual void IdleMode ();
		virtual void radio_init ();
		virtual void SetPower (u8 TXPower);
		virtual uint8_t GetRSSI ();
		virtual uint8_t GetPGA ();
		virtual uint32_t convert_to_bps (EDATRATE val);

};



#endif
