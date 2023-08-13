#include "RFM69.h"
#include "comonrut.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "hard_rut.h"


TRFM69::TRFM69 (ISPI *s, const uint16_t szz, const S_GPIOPIN *p) : c_trnsm_bufer_sizes (szz + 2), pins_ir (p)
{
SPIx = s;
framebuffer = new uint8_t [c_trnsm_bufer_sizes];
data_rx_ix = 0;
rfmode = ERFMODE_SLEEP;
isRFM69HW = 1;                     // if RFM69HW model matches high power enable possible
powerLevel = 31;
promiscuousMode = 0;
f_rx_frame_blocked = true;

Indik_Signal_rx = 0; Indik_Signal_tx = 0;
SYSBIOS::DEL_TIMER_ISR (&Indik_Signal_rx);
SYSBIOS::DEL_TIMER_ISR (&Indik_Signal_tx);
//F_EndTx = true;
//SYSBIOS::DEL_TIMER_ISR (&Timer_RxOn);
_pin_low_init_out_pp (const_cast <S_GPIOPIN*>(&pins_ir[ERFMPINS_RESET]), 1);
_pin_low_init_in (const_cast <S_GPIOPIN*>(&pins_ir[ERFMPINS_ISR]), 1);
AddObjectToExecuteManager ();
Init ();
}



IFCRFTX *TRFM69::create (ISPI *s, const uint16_t szz, const S_GPIOPIN *p)	// , IRFRX *obbj
{
	return new TRFM69 (s, szz, p); 
}



bool TRFM69::get_isr_pin ()
{
	return _pin_get (const_cast <S_GPIOPIN*>(&pins_ir[ERFMPINS_ISR]));
}



void TRFM69::ResetPinTo (bool vals_set)
{
	_pin_pp_to (const_cast <S_GPIOPIN*>(&pins_ir[ERFMPINS_RESET]), vals_set);
}



void TRFM69::ResetPulseToPin ()
{
ResetPinTo (0);
//SYSBIOS::Wait (60);
//ResetPinTo (1);
//SYSBIOS::Wait (1);
ResetPinTo (0);
//SYSBIOS::Wait (60);
}





uint8_t TRFM69::ReadFromFifo (uint8_t *dst, uint16_t maxsz)
{
uint8_t data_len, rv = 0;
SPIx->cs_0 ();
SPIx->txrx (REG_FIFO & 0x7F);
data_len = SPIx->txrx (0);
bool f_fifo_need_clr = false;
if (data_len && dst && maxsz) {
	if (data_len >= sizeof(S_RFFRGM_HDR_T)) {
		rv = data_len;
		while (data_len && maxsz)
			{
			*dst++ = SPIx->txrx(0);
			maxsz--;
			data_len--;
			}
		}
	else
		{
		f_fifo_need_clr = true;
		}
	}
else
	{
	f_fifo_need_clr = true;
	}
SPIx->cs_1 ();
if (f_fifo_need_clr)
	{
	ic_setMode (ERFMODE_STANDBY);
	ic_setMode (ERFMODE_RX);	// clear fifo
	}
return rv;
}


/*
uint8_t TRFM69::ReadData (uint8_t *lDst, uint8_t bufsize)
{
uint8_t rv = 0;
if (rx_size_present)
	{
	if (bufsize > rx_size_present) bufsize = rx_size_present;
	CopySDC_Data (recv_DATA, lDst, bufsize);
	rv = bufsize;
	}
return rv;
}
*/









/*
const uint8_t CONFIG[][2] = {
    { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },    // 0x01
    { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 },    // 0x02
    { REG_BITRATEMSB, RF_BITRATEMSB_300000 },    // 0x03
    { REG_BITRATELSB, RF_BITRATELSB_300000 },    // 0x04
    { REG_FDEVMSB, RF_FDEVMSB_300000 },    // 0x05
    { REG_FDEVLSB, RF_FDEVLSB_300000 },    // 0x06
    { REG_FRFMSB, RF_FRFMSB_915 },    // 0x07
    { REG_FRFMID, RF_FRFMID_915 },     // 0x08
    { REG_FRFLSB, RF_FRFLSB_915 },    // 0x09
    { REG_RXBW, RF_RXBW_DCCFREQ_111 | RF_RXBW_MANT_16 | RF_RXBW_EXP_0 },    // 0x19
    { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 },    // 0x25
    { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF },    //0x26
    { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN },    // 0x28
    { REG_RSSITHRESH, 220 },    // 0x29
    { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE },    // 0x2D
    { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },    // 0x2E
    { REG_SYNCVALUE1, 0x2D },    // 0x2F
    { REG_SYNCVALUE2, networkID },    // 0x30
    { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_OFF | RF_PACKET1_CRCAUTOCLEAR_OFF | RF_PACKET1_ADRSFILTERING_OFF },    // 0x37
    { REG_PAYLOADLENGTH, 66 },    // 0x38
    { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE },    // 0x3C
    { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF },    // 0x3D
    { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 },    // 0x6F
    { 255, 0 }
  };
*/


// freqBand must be selected from 315, 433, 868, 915
void TRFM69::Init ()		// uint8_t nodeID, uint8_t networkID
{
uint16_t freqBand = RF_868MHZ;
    const uint8_t CONFIG[][2] =
    {
        /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
        /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
        /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_300000}, // default: 4.8 KBPS RF_BITRATEMSB_115200 RF_BITRATEMSB_300000
        /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_300000},  // RF_BITRATELSB_300000
        /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_300000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
        /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_300000},

        //* 0x07 */ { REG_FRFMSB, RF_FRFMSB_433},
        //* 0x08 */ { REG_FRFMID, RF_FRFMID_433},
        //* 0x09 */ { REG_FRFLSB, RF_FRFLSB_433},
        
        /* 0x07 */ { REG_FRFMSB, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFMSB_315 : (freqBand==RF_433MHZ ? RF_FRFMSB_433 : (freqBand==RF_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
        /* 0x08 */ { REG_FRFMID, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFMID_315 : (freqBand==RF_433MHZ ? RF_FRFMID_433 : (freqBand==RF_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
        /* 0x09 */ { REG_FRFLSB, (uint8_t) (freqBand==RF_315MHZ ? RF_FRFLSB_315 : (freqBand==RF_433MHZ ? RF_FRFLSB_433 : (freqBand==RF_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },


        // looks like PA1 and PA2 are not implemented on RFM69W, hence the max output power is 13dBm
        // +17dBm and +20dBm are possible on RFM69HW
        // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
        // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
        // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
        ///* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
        ///* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 }, // over current protection (default is 95mA)

        // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
        /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_111 | RF_RXBW_MANT_16 | RF_RXBW_EXP_0 }, // (BitRate < 2 * RxBw) RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2
        //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
        /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
        /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
        /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
        /* 0x29 */ { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
        ///* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
        /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
        /* 0x2F */ { REG_SYNCVALUE1, 0x2D },      // attempt to make this compatible with sync1 byte of RFM12B lib
        /* 0x30 */ { REG_SYNCVALUE2, C_networkID }, // NETWORK ID
        /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
        /* 0x38 */ { REG_PAYLOADLENGTH, 66 }, // in variable length mode: the max frame size, not used in TX
        ///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
        /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
        /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
        //for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
        /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
        {255, 0}
    };
    
		
		SPIx->init ();
		ResetPulseToPin ();
		//InterruptPinActivate ();

    
    while (readReg(REG_SYNCVALUE1) != 0xaa)
    {
        writeReg(REG_SYNCVALUE1, 0xaa);
    }

    while (readReg(REG_SYNCVALUE1) != 0x55)
    {
        writeReg(REG_SYNCVALUE1, 0x55);
    }

    for (uint8_t i = 0; CONFIG[i][0] != 255; i++)
        writeReg(CONFIG[i][0], CONFIG[i][1]);

    encrypt(0);

    setHighPower(isRFM69HW);        // called regardless if it's a RFM69W or RFM69HW
    ic_setMode (ERFMODE_STANDBY);
    while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00);

    //setAddress(nodeID);            // setting this node id
    //setNetwork(C_networkID);
}


/*
void TRFM69::setAddress (uint8_t addr)
{
    writeReg(REG_NODEADRS, addr);
		//Dev_addres = addr;
}
*/



void TRFM69::setNetwork (uint8_t networkID)
{
    writeReg(REG_SYNCVALUE2, networkID);
		//Nwk_addres = networkID;
}



// set *transmit/TX* output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//       - for RFM69W the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//       - for RFM69HW the range is from 0-31 [5dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
void TRFM69::setPowerLevel(uint8_t powerLevel)
{
    uint8_t _powerLevel = powerLevel;
    if (isRFM69HW==1) _powerLevel /= 2;
    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0xE0) | _powerLevel);
}



//put transceiver in sleep mode to save battery - to wake or resume receiving just call receiveDone()
void TRFM69::sleep() 
{
    ic_setMode (ERFMODE_SLEEP);
}



uint8_t TRFM69::readTemperature(uint8_t calFactor) // returns centigrade
{
    ic_setMode (ERFMODE_STANDBY);
    writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
    while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
    return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
} // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction



// return the frequency (in Hz)
uint32_t TRFM69::getFrequency()
{
    return RF69_FSTEP * (((uint32_t) readReg(REG_FRFMSB) << 16) + ((uint16_t) readReg(REG_FRFMID) << 8) + readReg(REG_FRFLSB));
}



// set the frequency (in Hz)
void TRFM69::setFrequency (uint32_t freqHz)
{
    ERFMODE oldMode = rfmode;
    if (oldMode == ERFMODE_TX) ic_setMode (ERFMODE_RX);

    freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
    writeReg(REG_FRFMSB, freqHz >> 16);
    writeReg(REG_FRFMID, freqHz >> 8);
    writeReg(REG_FRFLSB, freqHz);
    if (oldMode == ERFMODE_RX) ic_setMode (ERFMODE_SYNT);
        
    ic_setMode (oldMode);
}



// Read byte from register
uint8_t TRFM69::readReg(uint8_t addr)
{
    SPIx->cs_0 ();
    SPIx->txrx(addr & 0x7F);
    uint8_t regval = SPIx->txrx(0);
    SPIx->cs_1 ();
    return regval;
}

// Write byte to register
void TRFM69::writeReg(uint8_t addr, uint8_t value)
{
    SPIx->cs_0 ();
    SPIx->txrx(addr | 0x80);
    SPIx->txrx(value);
    SPIx->cs_1 ();
}


unsigned char TRFM69::GetVersion ()
{
return readReg(REG_VERSION);
}



void TRFM69::encrypt(const char* key) 
{
    ic_setMode (ERFMODE_STANDBY);
    if (key)
    {
        SPIx->cs_0 ();
        SPIx->txrx(REG_AESKEY1 | 0x80);
        for (uint8_t i = 0; i < 16; i++)
            SPIx->txrx (key[i]);
        SPIx->cs_1 ();
    }
    else
        writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFE) | 0x00);    
}


ERFMODE TRFM69::getMode()
{
return rfmode;
}



void TRFM69::ic_setMode (ERFMODE newMode)
{
	if (newMode == rfmode) return;
	//while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) {}; // wait for ModeReady

    switch (newMode)
			{
			case ERFMODE_TX:
				{
				//if (newMode == mode) break;
				writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
				writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
				if (isRFM69HW) setHighPowerRegs(1);
				break;
				}
			case ERFMODE_RX:
				{
				//if (newMode == mode) break;
				writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
				writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
				if (isRFM69HW) setHighPowerRegs(0);
				break;
				}
			case ERFMODE_SYNT:
				{
				//if (newMode == mode) break;
				writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
				break;
				}
			case ERFMODE_STANDBY:
				{
				writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
				break;
				}
			case ERFMODE_SLEEP:
				{
				writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
				break;
				}
			default:
			return;
			}
		while ((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) {}; // wait for ModeReady
    rfmode = newMode;
}
    


// internal function
void TRFM69::setHighPowerRegs(uint8_t onOff)
{
    if(onOff==1)
    {
    writeReg(REG_TESTPA1, 0x5D);
    writeReg(REG_TESTPA2, 0x7C);
    }
    else
    {
        writeReg(REG_TESTPA1, 0x55);
        writeReg(REG_TESTPA2, 0x70);
    }
}
    
// for RFM69HW only: you must call setHighPower(1) after rfm69_init() or else transmission won't work
void TRFM69::setHighPower(uint8_t onOff) 
{
    isRFM69HW = onOff;
    writeReg(REG_OCP, isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);

    if (isRFM69HW == 1) // turning ON
        writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
    else
        writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | powerLevel); // enable P0 only
}




int16_t TRFM69::ic_readRSSI (uint8_t forceTrigger)
{
    int16_t rssi = 0;
    if (forceTrigger==1)
    {
        writeReg(REG_RSSICONFIG, RF_RSSI_START);
        while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); 
    }
    rssi = -readReg(REG_RSSIVALUE);
    rssi >>= 1;
    return rssi;
}



bool TRFM69::ic_sendFrame (void* buffer, uint16_t bufferSize, ERFMODE endsw_to)		// unsigned char DestAdrr
{
	bool rv = true;
	unsigned long millis_current;
  ic_setMode (ERFMODE_STANDBY); // turn off receiver to prevent reception while filling fifo
	ic_setMode (ERFMODE_TX);
  if (bufferSize > RF69_MAX_DATA_LEN) bufferSize = RF69_MAX_DATA_LEN;

    SPIx->cs_0 ();
    SPIx->txrx(REG_FIFO | 0x80);
    for (uint8_t i = 0; i < bufferSize; i++)
        SPIx->txrx(((uint8_t*) buffer)[i]);
    SPIx->cs_1 ();

    //ic_setMode (ERFMODE_TX);
    millis_current = SYSBIOS::GetTickCountLong();
		Indik_Signal_tx = C_LEDBLINK_TIME;
    while (get_isr_pin ()) // if (!get_isr_pin ())
			{
			if (((SYSBIOS::GetTickCountLong() - millis_current) >= RF69_TX_LIMIT_MS)) 
				{
				rv = false;
				break;
				}
			} 
	if (ERFMODE_TX != endsw_to) ic_setMode (endsw_to);
    //ic_setMode (ERFMODE_STANDBY);
		//F_EndTx = true;		
	return rv;
}




/*
void TRFM69::ic_sendFrame (void* buffer, uint16_t bufferSize)		// unsigned char DestAdrr
{
	unsigned long millis_current;
	//LastDestAdr = DestAdrr;
    ic_setMode (ERFMODE_STANDBY); // turn off receiver to prevent reception while filling fifo
    if (bufferSize > RF69_MAX_DATA_LEN) bufferSize = RF69_MAX_DATA_LEN;

    SPIx.CS_select ();
    SPIx.TransmitSPIData(REG_FIFO | 0x80);
		//SPIx.TransmitSPIData ((bufferSize + 3));		// + размер 
		//SPIx.TransmitSPIData (DestAdrr);			// + адресс dest
		//SPIx.TransmitSPIData (Dev_addres);		// + адресс src
    for (uint8_t i = 0; i < bufferSize; i++)
        SPIx.TransmitSPIData(((uint8_t*) buffer)[i]);
    SPIx.CS_unselect ();

		//F_EndTx = false;
    ic_setMode (ERFMODE_TX);
    millis_current = SYSBIOS::GetTickCountLong();
		Indik_Signal_tx = true;
    while (get_isr_pin ()) // if (!get_isr_pin ())
			{
			if (((SYSBIOS::GetTickCountLong() - millis_current) >= RF69_TX_LIMIT_MS)) break;
			} 
		
    ic_setMode (ERFMODE_STANDBY);
		//F_EndTx = true;
		
}
*/


void TRFM69::rcCalibration()
{
    writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
    while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}




void TRFM69::receiveBegin()
{
    if (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY) writeReg(REG_PACKETCONFIG2, (readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
    ic_setMode (ERFMODE_RX);
}

// 1  = disable filtering to capture all frames on network
// 0 = enable node/broadcast filtering to capture only frames sent to this/broadcast address
void TRFM69::ic_promiscuous(uint8_t onOff)
{
    promiscuousMode = onOff;
    if(promiscuousMode==0)
        writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | RF_PACKET1_ADRSFILTERING_NODE);
    else
        writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | RF_PACKET1_ADRSFILTERING_OFF);    
}



void TRFM69::decode_ts (uint8_t ts_in, EFRFGTYPE *lcod, uint8_t *l_seqn, bool *f_ack)
{
	uint8_t dat;
	if (lcod)
		{
		dat = ts_in;
		dat >>= 6; dat &= 3;
		*lcod = (EFRFGTYPE)dat;
		}
	if (l_seqn)
		{
		dat = ts_in;
		dat &= 0x1F;
		*l_seqn = dat;
		}
	if (f_ack)
		{
		if (ts_in & 0x20)
			{
			*f_ack = true;
			}
		else
			{
			*f_ack = false;
			}
		}
}



uint8_t TRFM69::code_ts (EFRFGTYPE ts, bool f_ack, uint8_t seqn)
{
uint8_t rv = ts;
rv = rv << 6;
if (f_ack) rv |= 0x2;
seqn &= 0x1F;
rv |= seqn;
return rv;
}





bool TRFM69::copy_to_frame_bufer (uint8_t *src, uint8_t sz)
{
	bool rv = false;
	if (src)
		{
		uint16_t rxemptysize = c_trnsm_bufer_sizes - data_rx_ix;
		if (sz)
			{
			if (sz <= rxemptysize)
				{
				CopySDC_Data (src, &framebuffer[data_rx_ix], sz);
				data_rx_ix += sz;
				rv = true;
				}
			}
		}
	return rv;
}



bool TRFM69::copy_from_frame_bufer (uint8_t *dst, uint8_t sz)
{
	bool rv = false;
	if (dst)
		{
		uint16_t txsize = c_trnsm_bufer_sizes - data_tx_ix;
		if (sz)
			{
			if (sz <= txsize)
				{
				CopySDC_Data (&framebuffer[data_tx_ix], dst , sz);
				data_tx_ix += sz;
				rv = true;
				}
			}
		}
	return rv;
}



void TRFM69::seq_inc (uint8_t &sqdata)
{
	uint8_t dat = sqdata;
	dat++; dat &= 0x1F;
	sqdata = dat;
}



uint16_t TRFM69::calculate_crc (uint8_t *src, uint16_t sz)
{
uint16_t crc16 = 0;
uint8_t dat;
uint16_t addval;
while (sz)
	{
	dat = *src++;
	switch (dat)
		{
		case 0:
			addval = 0x0105;
			break;
		case 0xFF:
			addval = 0x0307;
			break;
		default:
			addval += dat;
			break;
		}
	crc16 += addval;
	sz--;
	}
if (crc16 == 0) crc16 = 1;
if (crc16 == 0xFFFF) crc16 = 2;
return crc16;
}



void TRFM69::mode_sw_to_rx ()
{
	data_rx_ix = 0;
	ic_setMode (ERFMODE_STANDBY);
	ic_setMode (ERFMODE_RX);		// clear fifo
	f_rx_frame_blocked = true;	// only single frame or fist frame enabled
}




void TRFM69::Task ()
{
if (!get_isr_pin ())
	{
	switch ((char)rfmode)
		{
		case ERFMODE_RX:
			{
			uint16_t rxemptysize = c_trnsm_bufer_sizes - data_rx_ix;
			bool f_rx_ok = false;
			if (rxemptysize)
				{
				uint8_t currxsize = ReadFromFifo (tempbf, sizeof(tempbf));
				if (currxsize >= sizeof(S_RFFRGM_HDR_T) && currxsize <= sizeof(S_RFFRAGMENT_TAG))
					{
					S_RFFRAGMENT_TAG *frame = (S_RFFRAGMENT_TAG*)tempbf;
						
					if (frame->hdr.lsize && frame->hdr.lsize <= C_FRAGMENTDATA_SIZE)
						{
						uint16_t datacopy_size = frame->hdr.lsize;
						uint8_t *lsrccopy = tempbf + sizeof(S_RFFRGM_HDR_T);
						EFRFGTYPE fcod; uint8_t seqn;
						decode_ts (frame->hdr.ts_code, &fcod, &seqn, 0);
						switch (fcod)	
							{
							case EFRFGTYPE_MIDLE: 
								{
								if (f_rx_frame_blocked) break;
								if (seqn == last_rx_seq_n)
									{
									seq_inc (last_rx_seq_n);
									f_rx_ok = copy_to_frame_bufer (lsrccopy, datacopy_size);
									}
								else
									{
									f_rx_frame_blocked = true;
									}
								break;
								}
							case EFRFGTYPE_FIST:
								{
								if (!seqn)
									{
									last_rx_seq_n = seqn;
									seq_inc (last_rx_seq_n);	// следующий номер должен быть на 1 больше
									data_rx_ix = 0;
									f_rx_ok = copy_to_frame_bufer (lsrccopy, datacopy_size);
									f_rx_frame_blocked = false;
									}
								else
									{
									f_rx_frame_blocked = true;
									}
								break;
								}
							case EFRFGTYPE_LAST:
								{
								if (f_rx_frame_blocked) break;
								if (seqn == last_rx_seq_n)
									{
									if (copy_to_frame_bufer (lsrccopy, datacopy_size))
										{
										// проверка контрольной суммы
										if (data_rx_ix > C_FRAMECRC_SIZEOF)
											{
											uint16_t user_data_size = data_rx_ix - C_FRAMECRC_SIZEOF;
											uint16_t crc16 = calculate_crc (framebuffer, user_data_size);
											uint16_t *lcrc = (uint16_t*)&framebuffer[user_data_size];
											if (crc16 == *lcrc) if (eventsobj_cb) eventsobj_cb->RF_recv_cb (framebuffer, user_data_size, ic_readRSSI(0));
											}
										f_rx_ok = true;
										}
									}
								else
									{
									f_rx_frame_blocked = true;
									}
								break;
								}
							case EFRFGTYPE_SINGLE:
								{
								data_rx_ix = 0;
								f_rx_frame_blocked = false;
								if (copy_to_frame_bufer (lsrccopy, datacopy_size))
									{
									// проверка контрольной суммы
									if (data_rx_ix > C_FRAMECRC_SIZEOF)
										{
										uint16_t user_data_size = data_rx_ix - C_FRAMECRC_SIZEOF;
										uint16_t crc16 = calculate_crc (framebuffer, user_data_size);
										uint16_t *lcrc = (uint16_t*)&framebuffer[user_data_size];
										if (crc16 == *lcrc) if (eventsobj_cb) eventsobj_cb->RF_recv_cb (framebuffer, user_data_size, ic_readRSSI(0));
										}
									f_rx_ok = true;
									}
								break;
								}
							default: 
								{
								f_rx_frame_blocked = true;
								break;
								}
							}
						}
					}
				}
				
			if (f_rx_ok) Indik_Signal_rx = C_LEDBLINK_TIME;
			break;
			}
		case ERFMODE_TX:
			{
			if (c_full_size_tx > data_tx_ix)
				{
				uint16_t dlt_tx = c_full_size_tx - data_tx_ix;
				ERFMODE mod_aftetx;
				EFRFGTYPE ftype = EFRFGTYPE_MIDLE;
				uint16_t tx_size;
					
				if (dlt_tx <= C_FRAGMENTDATA_SIZE) 
					{
					if (!data_tx_ix) 
						{
						ftype = EFRFGTYPE_SINGLE;
						last_tx_seqn = 0;
						}
					else
						{
						ftype = EFRFGTYPE_LAST;
						}
					tx_size = dlt_tx;
					mod_aftetx = ERFMODE_RX;
					}
				else
					{
					if (!data_tx_ix) 
						{
						ftype = EFRFGTYPE_FIST;
						last_tx_seqn = 0;
						}
					tx_size = C_FRAGMENTDATA_SIZE;
					mod_aftetx = ERFMODE_TX;
					}
				
				S_RFFRAGMENT_TAG *frame = (S_RFFRAGMENT_TAG*)tempbf;
				frame->hdr.ts_code = code_ts (ftype, false, last_tx_seqn);
				frame->hdr.lsize = tx_size;
				seq_inc (last_tx_seqn);
				copy_from_frame_bufer (frame->data, tx_size);
				if (ic_sendFrame (tempbf, tx_size, mod_aftetx))
					{
					if (ftype == EFRFGTYPE_LAST || ftype == EFRFGTYPE_SINGLE)
						{
						if (eventsobj_cb) eventsobj_cb->RF_txend_cb (true);
						}
					}
				else
					{
					if (eventsobj_cb) eventsobj_cb->RF_txend_cb (false);
					mode_sw_to_rx ();
					}
				}
			else
				{
				mode_sw_to_rx ();
				}
			break;
			}
		}
	}
}



bool TRFM69::send (uint8_t *src, uint16_t sz)
{
	bool rv = false;
	if (src && sz)
		{
		if ((sz + C_FRAMECRC_SIZEOF) > c_trnsm_bufer_sizes) return false;
		CopySDC_Data (src, framebuffer, sz);
		uint16_t crc16 = calculate_crc (framebuffer, sz);
		c_full_size_tx = sz;
		*((uint16_t*)&framebuffer[c_full_size_tx]) = crc16;
		c_full_size_tx += C_FRAMECRC_SIZEOF;
		data_tx_ix = 0;
		last_tx_seqn = 0;
		rv = true;
		}
	return rv;
}




/*
void TRFM69::Recive_On (unsigned short time_on)
{
if (time_on == 0xFFFF)
	{
	
	}
receiveBegin ();

//F_RxActive = false;
//SYSBIOS::DEL_TIMER_ISR (&Timer_RxOn);
}
*/




// Функция для координатора (MASTER режима)
// передает комманду и ждет ответа отведенное время (обычно время слота)
/*
ETRSSTAT TRFM69::MasterSlotTransaction (LPBUFR *lTxData, LPBUFR *lRxData, unsigned short beacontimesymb)
{
ETRSSTAT rv = ETRS_ERROR;

if (lTxData->sizes)
	{
	unsigned short curLtime = TBEACONSYS::TimerGet (), delt_t;
	sendFrame (lTxData->lRam, lTxData->sizes);
	ClearRecvStatus ();
	receiveBegin ();
	rv = ETRS_SUCCESS;
	while (!F_RxDataPresent)
		{
		delt_t = TBEACONSYS::TimerGet () - curLtime;
		if (delt_t >= beacontimesymb)
			{
			rv = ETRS_TIMEOUT;
			break;
			}
		ReceiveDataToBufer ();
		}
	if (rv == ETRS_SUCCESS) 
		{
		unsigned short sizes = lRxData->sizes;
		if (sizes > sizeof(recv_DATA)) sizes = sizeof(recv_DATA);
		CopySDC_Data (recv_DATA, lRxData->lRam, sizes);
		}
	}
return rv;
}
*/





bool TRFM69::GetIndicatorRx ()
{
return (Indik_Signal_rx)?true:false;
}



bool TRFM69::GetIndicatorTx ()
{
	return (Indik_Signal_tx)?true:false;
}




