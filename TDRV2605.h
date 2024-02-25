#ifndef _H_STM32_DRV2605_H_
#define _H_STM32_DRV2605_H_


#include "I2CSOFTWARE.H"
#include "TFTASKIF.h"
#include "SYSBIOS.H"
#include "hard_rut.h"


enum EERM {EERM_NONE = 0, EERM_EFF_1, EERM_EFF_2, EERM_EFF_3, EERM_ENDENUM};

#define DRV2605_ADDR 0x5A ///< Device I2C address

#define DRV2605_REG_STATUS 0x00       ///< Status register
#define DRV2605_REG_MODE 0x01         ///< Mode register
#define DRV2605_MODE_INTTRIG 0x00     ///< Internal trigger mode
#define DRV2605_MODE_EXTTRIGEDGE 0x01 ///< External edge trigger mode
#define DRV2605_MODE_EXTTRIGLVL 0x02  ///< External level trigger mode
#define DRV2605_MODE_PWMANALOG 0x03   ///< PWM/Analog input mode
#define DRV2605_MODE_AUDIOVIBE 0x04   ///< Audio-to-vibe mode
#define DRV2605_MODE_REALTIME 0x05    ///< Real-time playback (RTP) mode
#define DRV2605_MODE_DIAGNOS 0x06     ///< Diagnostics mode
#define DRV2605_MODE_AUTOCAL 0x07     ///< Auto calibration mode

#define DRV2605_REG_RTPIN 0x02    ///< Real-time playback input register
#define DRV2605_REG_LIBRARY 0x03  ///< Waveform library selection register
#define DRV2605_REG_WAVESEQ1 0x04 ///< Waveform sequence register 1
#define DRV2605_REG_WAVESEQ2 0x05 ///< Waveform sequence register 2
#define DRV2605_REG_WAVESEQ3 0x06 ///< Waveform sequence register 3
#define DRV2605_REG_WAVESEQ4 0x07 ///< Waveform sequence register 4
#define DRV2605_REG_WAVESEQ5 0x08 ///< Waveform sequence register 5
#define DRV2605_REG_WAVESEQ6 0x09 ///< Waveform sequence register 6
#define DRV2605_REG_WAVESEQ7 0x0A ///< Waveform sequence register 7
#define DRV2605_REG_WAVESEQ8 0x0B ///< Waveform sequence register 8

#define DRV2605_REG_GO 0x0C         ///< Go register
#define DRV2605_REG_OVERDRIVE 0x0D  ///< Overdrive time offset register
#define DRV2605_REG_SUSTAINPOS 0x0E ///< Sustain time offset, positive register
#define DRV2605_REG_SUSTAINNEG 0x0F ///< Sustain time offset, negative register
#define DRV2605_REG_BREAK 0x10      ///< Brake time offset register
#define DRV2605_REG_AUDIOCTRL 0x11  ///< Audio-to-vibe control register
#define DRV2605_REG_AUDIOLVL                                                   \
  0x12 ///< Audio-to-vibe minimum input level register
#define DRV2605_REG_AUDIOMAX                                                   \
  0x13 ///< Audio-to-vibe maximum input level register
#define DRV2605_REG_AUDIOOUTMIN                                                \
  0x14 ///< Audio-to-vibe minimum output drive register
#define DRV2605_REG_AUDIOOUTMAX                                                \
  0x15                          ///< Audio-to-vibe maximum output drive register
#define DRV2605_REG_RATEDV 0x16 ///< Rated voltage register
#define DRV2605_REG_CLAMPV 0x17 ///< Overdrive clamp voltage register
#define DRV2605_REG_AUTOCALCOMP                                                \
  0x18 ///< Auto-calibration compensation result register
#define DRV2605_REG_AUTOCALEMP                                                 \
  0x19                            ///< Auto-calibration back-EMF result register
#define DRV2605_REG_FEEDBACK 0x1A ///< Feedback control register
#define DRV2605_REG_CONTROL1 0x1B ///< Control1 Register
#define DRV2605_REG_CONTROL2 0x1C ///< Control2 Register
#define DRV2605_REG_CONTROL3 0x1D ///< Control3 Register
#define DRV2605_REG_CONTROL4 0x1E ///< Control4 Register
#define DRV2605_REG_VBAT 0x21     ///< Vbat voltage-monitor register
#define DRV2605_REG_LRARESON 0x22 ///< LRA resonance-period register

#define DRV2605_POWER_INIT_TIME 5 	// 5 ms

typedef struct {
	uint8_t val;
	uint16_t time;
} S_VIBRPOINT_T;


enum ESHOCKSW {ESHOCKSW_NONE = 0, ESHOCKSW_PIN_EN, ESHOCKSW_INIT, ESHOCKSW_GO, ESHOCKSW_STOP, ESHOCKSW_ENDENUM};
#define C_DRVINIT_FUNCT_CNT 13

class TDRV2605: public TFFC {
		virtual void Task () override;
		S_GPIOPIN *en_pin;
		const uint8_t c_chip_adr;
		TI2CIFACE *i2c;
		ESHOCKSW sw;
		SYSBIOS::Timer relax_timer;
		S_VIBRPOINT_T *curprog_list;

		
		typedef bool (TDRV2605::*init_tile_funct) ();
		uint8_t init_cmd_ix;
		bool init_funct_mode ();
		bool init_funct_rtpin ();
		bool init_funct_ws_0 ();
		bool init_funct_ws_1 ();
		bool init_funct_overdrv ();
		bool init_funct_systain_pos ();
		bool init_funct_systain_neg ();
		bool init_funct_break ();
		bool init_funct_audio_max ();
		bool init_funct_time_pbck ();
		bool init_funct_control3 ();
		bool init_funct_libdef ();
		bool init_funct_modedef ();
		init_tile_funct initarr[C_DRVINIT_FUNCT_CNT];
		
	
		bool writeRegister8 (uint8_t reg, uint8_t val);
		bool readRegister8 (uint8_t reg, uint8_t &dat);
	

		bool init ();
		bool setWaveform(uint8_t slot, uint8_t w);
		bool selectLibrary(uint8_t lib);
		bool go(void);
		bool stop(void);
		bool setMode (uint8_t mode);
		bool setRealtimeValue(uint8_t rtp);
		bool useERM();
		bool useLRA();
	
	protected:
		
		uint8_t init_ok_cntr;
		
	public:
		TDRV2605 (TI2CIFACE *ifc, S_GPIOPIN *p);
		void Effect (EERM ef);
		
};

#endif


