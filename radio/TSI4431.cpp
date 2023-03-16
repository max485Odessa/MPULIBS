#include "TSI4431.h"
#include "hard_rut.h"
#include "resources.h"
#include "stm32f10x_exti.h"
#include "misc.h"


static const uint8_t modem_regs[] =   {0x6E, 0x6F, 0x70, 0x58,   0x72, 0x71,   0x1C, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x1D, 0x1E, 0x2A, 0x1F, 0x69, 0};		// << 0 конец списка регистров
	
static const uint8_t preset_7200[] =  {0x3A, 0xFB, 0x2C, 0x80,   0xA0, 0x23,   0x9A, 0x41, 0x60, 0x27, 0x52, 0x00, 0x08, 0x44, 0x0A, 0x41, 0x03, 0x60};
static const uint8_t preset_9600[] =  {0x4E, 0xA5, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0x71, 0x40, 0x34, 0x6E, 0x00, 0x0C, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_12000[] = {0x62, 0x4E, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0xF4, 0x20, 0x41, 0x89, 0x00, 0x12, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_14400[] = {0x75, 0xF7, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x19, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_16800[]	= {0x89, 0xA0, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0x65, 0x20, 0x5B, 0xC0, 0x00, 0x21, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_19200[] = {0x9D, 0x49, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0x39, 0x20, 0x68, 0xDC, 0x00, 0x2A, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_21600[] = {0xB0, 0xF2, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0x16, 0x20, 0x75, 0xF7, 0x00, 0x35, 0x44, 0x0A, 0x48, 0x03, 0x60};
static const uint8_t preset_24000[] = {0xC4, 0x9C, 0x2C, 0x80,   0xA0, 0x23,   0x9B, 0xFA, 0x00, 0x83, 0x12, 0x00, 0x41, 0x44, 0x0A, 0x48, 0x03, 0x60};

static const uint8_t *modem_speedsets[EDATRATE_ENDENUM] = {preset_7200, preset_9600, preset_12000, preset_14400, preset_16800, preset_19200, preset_21600, preset_24000};



void TRADIOIF::preinit_set_baudrate (EDATRATE val)
{
	cur_datarate = val;
}



void TRADIOIF::preinit_set_power (uint8_t val)
{
	cur_rf_power = val;
}




TSI4431Class::TSI4431Class ( const S_SPIPINS_T *spipns, float strt_f_mhz, float stp_f_mhz, float ch_f_mhz) 
: c_freq_start_band_Mhz(strt_f_mhz), 
c_freq_stop_band_Mhz (stp_f_mhz), 
c_freq_chann_width_Mhz (ch_f_mhz), 
c_chann_amount (((c_freq_stop_band_Mhz - c_freq_start_band_Mhz)) / c_freq_chann_width_Mhz)
{
//fifo_RX = new TTFIFO<S_RAWRADIO_CAPSUL_T> (ff_rx_cnt);	
//fifo_TX = new TTFIFO<S_RAWRADIO_CAPSUL_T> (ff_tx_cnt);	
spi = new TSPI (spipns);
_pin_low_init_in (&rf_irqpin, 1);
AddObjectToExecuteManager ();
current_freq_ofs = 0;
f_header_en = false;
}



uint32_t TSI4431Class::convert_to_bps (EDATRATE val)
{
	if (val >= EDATRATE_ENDENUM) val = EDATRATE_24000;
	cur_datarate = val; 
	return  7200 + (2400 * val);
}





// в мегагерцах
void TSI4431Class::set_freq_raw (float val_mhz)
{
		uint8_t fb;
		uint16_t fc;
	int32_t khz = val_mhz * 1000;
		if(khz < 480000) {
			fb = (khz - 240000) / 10000;
			fc = (4000 * khz) / 625 - 64000 * (uint32_t)fb - 1536000;
		} else {
			fb = (khz - 480000) / 20000;
			fc = (4000 * khz) / 1250 - 64000 * (uint32_t)fb - 1536000;
			fb |= 0x20;
		}
		
		WriteRegister(0x75, 0x40 | fb); // (sbse = 1, hbsel = ?, fb = ?)
		WriteRegister(0x76, (uint8_t)(fc >> 8)); // (fc[15:8] = ?)
		WriteRegister(0x77, (uint8_t)fc); // (fc[7:0] = ?)
		
}




uint8_t TSI4431Class::GetRSSI ()
{
return ReadRegister (0x26);	
}



uint8_t TSI4431Class::GetPGA ()
{
return (ReadRegister (0x69) & 0x0F) * 3;	
}





void TSI4431Class::mux_tx_mode ()
{
	WriteRegister(0x0E, 2 );						// GPIO0 = 0(tx_on), GPIO1 - 1(rx_on)
}



void TSI4431Class::mux_rx_mode ()
{
WriteRegister(0x0E, 1 );						// GPIO0 = 1(tx_on), GPIO1 - 0(rx_on)
}



void TSI4431Class::mux_idle_mode ()
{
WriteRegister(0x0E, 3 );						// GPIO0 = 1, GPIO1 - 1
}


/*
void TSI4431Class::SetBps (uint32_t val)
{
	float falf = val;
	float muldd;
	uint8_t datmd;
	if (val < 30000)
		{
		muldd = 2.097152;
		datmd = 0x2D;
		}
	else
		{
		muldd = 0.065536;
		datmd = 0x0D;
		}
	falf *= muldd;
	uint16_t vall16 = falf;
	
	WriteRegister (0x6E, vall16 >> 8);
	WriteRegister (0x6F, vall16 & 0xFF);
	WriteRegister (0x70, datmd);
}
*/






void TSI4431Class::SetModemSpeed (EDATRATE val)
{
uint8_t *arreg = const_cast<uint8_t*>(modem_regs);
uint8_t reg;
if (val >= EDATRATE_ENDENUM) val = EDATRATE_24000;
uint8_t *arvalues = const_cast<uint8_t*>(modem_speedsets[val]);
while ((reg = *arreg++)) WriteRegister (reg, *arvalues++);
}


/*
void TSI4431Class::SetDeviation (float hz)
{
uint8_t reg = hz / 625.0;
WriteRegister (0x72, reg);
}
*/



void TSI4431Class::SetPower (u8 TXPower)
{
	WriteRegister (0x6D, (TXPower | 0x18) & 0x1F);
}



void TSI4431Class::radio_init ()
{	
	u8 ItStatus1, ItStatus2;
	  //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ
      EZ_RSTATUS(ItStatus1, ItStatus2);
      //??????????? ?????
      EZ_SWRST() ;    //?????? ???????? 0x80 ? ??????? Operating & Function Control1
      // ???? ?????? ?????????? POR (????? ??? ?????? ???????) ?? ???????????
      EZ_WAIT_NIRQ ();	        
      //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2) ; 
      //put_messageP(PSTR("\r\nRadio is ready"));   
      // ???? ?????? ?????????? "??????????" ?? ???????????        
      EZ_WAIT_NIRQ ();                 
      //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2);  
		
		WriteRegister(0x09, 0x7F); // c = 12.5p
		SetPower (cur_rf_power);
		//WriteRegister(0x6D, 0x0F); // set power max power
		//rfm23_write(0x34, 64); // 64 nibbles = 32byte preamble
		//WriteRegister(0x34, 10); // 10 nibbles
		
		WriteRegister(0x05, 6); // interrupt on ipksent, ipkvalid  tx & rx
		WriteRegister(0x06, 0); // no other interrupt
		
		WriteRegister(0x27, 0x4A); // max background noise level: -78dBm
		
		SetModemSpeed (cur_datarate);
		
		// 57.6 kbps
		//WriteRegister(0x1C, 0x03); // (dwn3_bypass = 0, ndec_exp = 0, filset = 3)
		//WriteRegister(0x1D, 0x40); // (afcbd = 0, enafc = 1, afcgearh = 0, afcgearl = 0)
		//WriteRegister(0x20, 0x45); // (rxosr[7:0] = 0x45)
		//WriteRegister(0x21, 0x01); // (rxosr[10:8] = 0x00, stallctrl = 0, ncoff[19:16] = 1)
		//WriteRegister(0x22, 0xD7); // (ncoff[15:8] = 0xD7)
		//WriteRegister(0x23, 0xDC); // (ncoff[7:0] = 0xDC)
		//WriteRegister(0x24, 0x07); // (crgain[10:8] = 0x07)
		//WriteRegister(0x25, 0x6E); // (crgain[7:0] = 0x6E)
		
		//SetBps (57600); 
		//WriteRegister(0x6E, 0x0E); // (txdr[15:8] = 0x0E) SetBps
		//WriteRegister(0x6F, 0xBF); // (txdr[7:0] = 0xBF) SetBps
		//WriteRegister(0x70, 0x0D); // (txdtrtscale = 0, enphpwdn = 0, manppol = 1, enmaninv = 1, enmanch = 0, enwhite = 1) SetBps
		
		//WriteRegister(0x71, 0x23); // (trclk = 0, dtmod = 2, eninv = 0, fd[8] = 0, modtyp = 3)
		//WriteRegister(0x72, 0x2E); // (fd[7:0] = 0x2E)
		
		set_freq_raw (430.4);
		//WriteRegister(0x75, 0x40 | fb); // (sbse = 1, hbsel = ?, fb = ?)
		//WriteRegister(0x76, (uint8_t)(fc >> 8)); // (fc[15:8] = ?)
		//WriteRegister(0x77, (uint8_t)fc); // (fc[7:0] = ?)
		
		// 100 kbps
		/*rfm23_write(0x1C, 0x8F); // (dwn3_bypass = 1, ndec_exp = 0, filset = F)
		rfm23_write(0x1D, 0x40); // (afcbd = 0, enafc = 1, afcgearh = 0, afcgearl = 0)
		rfm23_write(0x20, 0x78); // (rxosr[7:0] = 0x78)
		rfm23_write(0x21, 0x01); // (rxosr[10:8] = 0x00, stallctrl = 0, ncoff[19:16] = 1)
		rfm23_write(0x22, 0x11); // (ncoff[15:8] = 0x11)
		rfm23_write(0x23, 0x11); // (ncoff[7:0] = 0x11)
		rfm23_write(0x24, 0x04); // (crgain[10:8] = 0x04)
		rfm23_write(0x25, 0x46); // (crgain[7:0] = 0x46)
		rfm23_write(0x6E, 0x19); // (txdr[15:8] = 0x19)
		rfm23_write(0x6F, 0x9A); // (txdr[7:0] = 0x9A)
		rfm23_write(0x70, 0x0D); // (txdtrtscale = 0, enphpwdn = 0, manppol = 1, enmaninv = 1, enmanch = 0, enwhite = 1)
		rfm23_write(0x71, 0x23); // (trclk = 0, dtmod = 2, eninv = 0, fd[8] = 0, modtyp = 3)
		rfm23_write(0x72, 0x50); // (fd[7:0] = 0x50)
		rfm23_write(0x75, 0x73); // (sbse = 1, hbsel = 1, fb = 19)
		rfm23_write(0x76, 0x67); // (fc[15:8] = 103)
		rfm23_write(0x77, 0xC0); // (fc[7:0] = 192)*/
		
		//WriteRegister(0x30, 0x8D);
		
		//WriteRegister(0x69, 0x20);
		
		WriteRegister(0x32, 0x88); // enable broadcast for header 3 ??, enable header 3 check ??
		WriteRegister(0x34, 0x0A); // 10 nibbles
		WriteRegister(0x35, 0x2A);	// ????	
		WriteRegister(0x33, 0x02);	// ????
		WriteRegister(0x30, 0x8D);	// ????
		//nodeId = node_id;
		//WriteRegister(0x43, 0x3F); // header enable 3
		WriteRegister(0x36, 0x2D);			// synchro[0]
		WriteRegister(0x37, 0xD4);		// synchro[1]
		
			WriteRegister(0x0B, 10 | 0xC0);															//write 0x12 to the GPIO0 Configuration(set the TX state)
			WriteRegister(0x0C, 10 | 0xC0 );															//write 0x15 to the GPIO1 Configuration(set the RX state) 
			
		//WriteRegister(0x0B, 0x12); // GPIO0: TX State (output)
		//WriteRegister(0x0C, 0x15); // GPIO1: RX State (output)
		WriteRegister(0x08, 0x20);
		
	WriteRegister(0x3A, 10);
	WriteRegister(0x3B, 10);
	WriteRegister(0x3C, 10);
	WriteRegister(0x3D, 10);
	
	WriteRegister(0x3F, 10); 
	WriteRegister(0x40, 10);
	WriteRegister(0x41, 10);
	WriteRegister(0x42, 10);
		
		EZ_RSTATUS(ItStatus1, ItStatus2);

		
	FifoReset ();
#ifdef DRAG_TRANSMITER
	//TxMode ();
	IdleMode ();
#else
	RxMode ();
#endif

//		RxMode(); // RX mode
		
		
	
	
	/*
	u8 ItStatus1, ItStatus2;
	  //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ
      EZ_RSTATUS(ItStatus1, ItStatus2);
      //??????????? ?????
      EZ_SWRST() ;    //?????? ???????? 0x80 ? ??????? Operating & Function Control1
      // ???? ?????? ?????????? POR (????? ??? ?????? ???????) ?? ???????????
      EZ_WAIT_NIRQ ();	        
      //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2) ; 
      //put_messageP(PSTR("\r\nRadio is ready"));   
      // ???? ?????? ?????????? "??????????" ?? ???????????        
      EZ_WAIT_NIRQ ();                 
      //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2);  
			//set the center frequency to 430 MHz
			set_freq_raw (430.4);		// c_freq_start_band_Mhz + (c_freq_stop_band_Mhz - c_freq_start_band_Mhz)/2
			//set the desired TX data rate (9.6kbps)
			SetBps (9600);
      //????????? ???????? ???????? ??????? ??? ???????? (+-45kHz)
      WriteRegister(0x72, 0x48);		// 0x48
      //????????? ?????????? ?????? ???????? xls ?????(9.6 kbps, deviation: 45 kHz, channel filter BW: 102.2 kHz)


		// 57.6 kbps
		WriteRegister(0x1C, 0x03); // (dwn3_bypass = 0, ndec_exp = 0, filset = 3)
		WriteRegister(0x1D, 0x40); // (afcbd = 0, enafc = 1, afcgearh = 0, afcgearl = 0)
		WriteRegister(0x20, 0x45); // (rxosr[7:0] = 0x45)
		WriteRegister(0x21, 0x01); // (rxosr[10:8] = 0x00, stallctrl = 0, ncoff[19:16] = 1)
		WriteRegister(0x22, 0xD7); // (ncoff[15:8] = 0xD7)
		WriteRegister(0x23, 0xDC); // (ncoff[7:0] = 0xDC)
		WriteRegister(0x24, 0x07); // (crgain[10:8] = 0x07)
		WriteRegister(0x25, 0x6E); // (crgain[7:0] = 0x6E)
		WriteRegister(0x6E, 0x0E); // (txdr[15:8] = 0x0E)
		WriteRegister(0x6F, 0xBF); // (txdr[7:0] = 0xBF)
		WriteRegister(0x70, 0x0D); // (txdtrtscale = 0, enphpwdn = 0, manppol = 1, enmaninv = 1, enmanch = 0, enwhite = 1)
		WriteRegister(0x71, 0x23); // (trclk = 0, dtmod = 2, eninv = 0, fd[8] = 0, modtyp = 3)
		WriteRegister(0x72, 0x2E); // (fd[7:0] = 0x2E)
		//WriteRegister(0x75, 0x40 | fb); // (sbse = 1, hbsel = ?, fb = ?)
		//WriteRegister(0x76, (uint8_t)(fc >> 8)); // (fc[15:8] = ?)
		//WriteRegister(0x77, (uint8_t)fc); // (fc[7:0] = ?)
		
		
      //????? ????????? 5 ????
      WriteRegister(0x34, 0x0A);
      //????? ??????????? ????????? 20 ???
      WriteRegister(0x35, 0x2A);	
      //?????? ???????????? ?????? (header bytes), ????????? ?????????? ????? ???????, ????????? ????? ??????????? ? 2 ?????
      WriteRegister(0x33, 0x02);	
      //?????? ??????????? 0x2DD4
      WriteRegister(0x36, 0x2D);
      WriteRegister(0x37, 0xD4);
      //?????????? ???????????? ??????? TX ? RX ? ?????? ?? CRC-16 (IBM)
      WriteRegister(0x30, 0x8D);
      //?????? ???????? ???????????? ???? ??? ??????
      WriteRegister(0x32, 0x00);
      //????????? ?????? FIFO ? GFSK ?????????
      WriteRegister(0x71, 0x63);		// 63

			
			WriteRegister(0x0B, 10 | 0xC0);															//write 0x12 to the GPIO0 Configuration(set the TX state)
			WriteRegister(0x0C, 10 | 0xC0 );															//write 0x15 to the GPIO1 Configuration(set the RX state) 
			//WriteRegister(0x0D, 10 | 0xC0);						// GPIO2 dig out			
			
			//WriteRegister(0x0B, 0x12);
      //WriteRegister(0x0C, 0x15);	
			
      //WriteRegister(0x0C, 0x12);
      //WriteRegister(0x0D, 0x15);	
      //???????????? ???
      WriteRegister(0x69, 0x60);
	  //????????? ???????? ???????????
	  WriteRegister(0x6D, (TXPower | 0x18) & 0x1F);
      //???????????? ?????????????
      WriteRegister(0x09, 0xD7);

      //WriteRegister(0x07, 0x05);
      //???????? 2 ??????????:
      // a) ?????? ?????????? ????? ??????????? ??????:  'ipkval'
      // ?) ?????? ?????????? ????? ?????? ? ???????????? ?? CRC:  'icrcerror' 
      //WriteRegister(0x05, 0x03);
      //WriteRegister(0x06, 0x00);
      //?????? ???????? ????????? ?????????? ??? ??????? ?????? ?????????? ? ???????????? ?????? NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2);	
			
			
	FifoReset ();
#ifdef DRAG_TRANSMITER
	//TxMode ();
	IdleMode ();
#else
	RxMode ();
#endif

*/
	
/*
	u8 ItStatus1, ItStatus2;
	
ItStatus1 = ReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = ReadRegister(0x04);													//read the Interrupt Status2 register

	//SW reset   
   WriteRegister (0x07, 0x80);															//write 0x80 to the Operating & Function Control1 register 

	//wait for POR interrupt from the radio (while the nIRQ pin is high)
	EZ_WAIT_NIRQ ();  
	//read interrupt status registers to clear the interrupt flags and release NIRQ pin
	ItStatus1 = ReadRegister(0x03);													//read the Interrupt Status1 register
	ItStatus2 = ReadRegister(0x04);													//read the Interrupt Status2 register

	//wait for chip ready interrupt from the radio (while the nIRQ pin is high)
	//EZ_WAIT_NIRQ (); 
	//read interrupt status registers to clear the interrupt flags and release NIRQ pin
	//ItStatus1 = ReadRegister(0x03);													//read the Interrupt Status1 register
	//ItStatus2 = ReadRegister(0x04);													//read the Interrupt Status2 register


  //WriteRegister(RFM22B_XTALCAP, 0x7F);   // XTAL cap = 12.5pF
  //WriteRegister(RFM22B_MCUCLK, 0x05);    // 2MHz clock
	//set the center frequency to 430 MHz
	set_freq_raw (c_freq_start_band_Mhz + (c_freq_stop_band_Mhz - c_freq_start_band_Mhz)/2);
	//WriteRegister(0x75, 0x75);															//write 0x75 to the Frequency Band Select register             
	//WriteRegister(0x76, 0xBB);															//write 0xBB to the Nominal Carrier Frequency1 register
	//WriteRegister(0x77, 0x80);  														//write 0x80 to the Nominal Carrier Frequency0 register
	
	//set the desired TX data rate (9.6kbps)
	SetBps (9600);
	//WriteRegister(0x6E, 0x4E);															//write 0x4E to the TXDataRate 1 register
	//WriteRegister(0x6F, 0xA5);															//write 0xA5 to the TXDataRate 0 register
	//WriteRegister(0x70, 0x2C);															//write 0x2C to the Modulation Mode Control 1 register
	
	//set the desired TX deviatioin (+-45 kHz)
	WriteRegister(0x72, 0x48);															//write 0x48 to the Frequency Deviation register 

	

	//set the preamble length to 5bytes  
	WriteRegister(0x34, 0x0A);															//write 0x0A to the Preamble Length register

	//Disable header bytes; set variable packet length (the length of the payload is defined by the
	//received packet length field of the packet); set the synch word to two bytes long
	WriteRegister(0x33, 0x02);															//write 0x02 to the Header Control2 register    
	
	//Set the sync word pattern to 0x2DD4
	WriteRegister(0x36, 0x2D);															//write 0x2D to the Sync Word 3 register
	WriteRegister(0x37, 0xD4);															//write 0xD4 to the Sync Word 2 register

	//enable the TX packet handler and CRC-16 (IBM) check
	WriteRegister(0x30, 0x0D);															//write 0x0D to the Data Access Control register
	//enable FIFO mode and GFSK modulation
	WriteRegister(0x71, 0x63);															//write 0x63 to the Modulation Mode Control 2 register  (dtmod=2=fifo,  )


	
  WriteRegister(0x0B, 10 | 0xC0);															//write 0x12 to the GPIO0 Configuration(set the TX state)
	WriteRegister(0x0C, 10 | 0xC0 );															//write 0x15 to the GPIO1 Configuration(set the RX state) 
	WriteRegister(0x0D, 10 | 0xC0);						// GPIO2 dig out
	//WriteRegister(0x0E, 4 );						// GPIO2 dig out
	//mux_rx_mode ();


	//set VCO and PLL
	//WriteRegister(0x57, 0x01);															//write 0x01 to the Chargepump Test register													
	//WriteRegister(0x58, 0xC0);															//write 0xC0 to the Chargepump Current Trimming/Override register
	//WriteRegister(0x59, 0x00);															//write 0x00 to the Divider Current Trimming register 	
	//WriteRegister(0x5A, 0x01); 														//write 0x01 to the VCO Current Trimming register 	//set Crystal Oscillator Load Capacitance register
	//set Crystal Oscillator Load Capacitance register
	WriteRegister(0x09, 0xD7);															//write 0xD7 to the CrystalOscillatorLoadCapacitance register
	//WriteRegister (0x07, 0x01);
	
	//WriteRegister(RFM22B_FREQOFF1, 0x00);    // no offset
  //WriteRegister(RFM22B_FREQOFF2, 0x00);    // no offset
  //WriteRegister(RFM22B_FHCH,        0x00);   // set to hop channel 0
	
	FifoReset ();

	IdleMode ();
	
	*/

}



void TSI4431Class::FifoReset ()
{
	WriteRegister (0x08, 0x02);
	WriteRegister (0x08, 0x00);		
}



ERDSTATUS TSI4431Class::GetReadStatus ()
{
ERDSTATUS rv = ERDSTATUS_READ_NONE;// ERDSTATUS_READ_ERROR;
volatile u8 ItStatus1, ItStatus2;
	
ItStatus1 = ReadRegister(0x03);
ItStatus2 = ReadRegister(0x04);		

do 	{
		if (ItStatus1 & 1)
			{
			rv = ERDSTATUS_READ_ERROR;
			break;
			}
		if (ItStatus1 & 2)
			{
			rv = ERDSTATUS_READ_OK;
			break;
			}
		}	while (false);
return rv;	
}



EWRSTATUS TSI4431Class::GetWriteStatus ()
{
EWRSTATUS rv = EWRSTATUS_WRITE_OK;
volatile u8 ItStatus1, ItStatus2;
	
ItStatus1 = ReadRegister(0x03);
ItStatus2 = ReadRegister(0x04);		
	
return rv;	
}

			



bool TSI4431Class::NIRQ ()
{
	bool rv = false;
	if (rf_irqpin.port->IDR & rf_irqpin.pin) rv = true;
	return rv;
}



void TSI4431Class::WriteRegister (u8 reg, u8 value)
{
	spi->cs_0 ();
	spi->transact (reg | 0x80);
	spi->transact (value);
	spi->cs_1 ();
}



u8 TSI4431Class::ReadRegister (u8 reg)
{
	u8 rv;
	spi->cs_0 ();
	spi->transact (reg);
	rv = spi->transact (0xFF);
	spi->cs_1 ();
	return rv;
}



void TSI4431Class::rfm23_burst (bool write, uint8_t address, uint8_t *data, uint8_t n) 
{
	if(n == 0) return;
	spi->cs_0 ();
	
	if(write) address |= 0x80;
	spi->transact (address);

	if(write)
		{
			do
				{
				spi->transact (*data++);
				} while(--n);
		}
	else
		{
		do
			{
			*(data++) = spi->transact (0);
			} while(--n);
		}
	
	spi->cs_1 ();
}




void TSI4431Class::EZ_RSTATUS (u8 &st1, u8 &st2)
{
	st1 = ReadRegister (3);
	st2 = ReadRegister (4);
}



void TSI4431Class::EZ_SWRST ()
{
	WriteRegister (0x07, 0x80);
}



void TSI4431Class:: EZ_WAIT_NIRQ ()
{
while (NIRQ ()) {};
}



void TSI4431Class::set_header_raw (S_SI4431HDR_T *val)
{
}






void TSI4431Class::Task ()
{
}



void TSI4431Class::SetFreq (float fr)
{
	set_freq_raw (fr);
	current_freq_val = fr;
}



void TSI4431Class::SetFreqOffset (long ofs)
{
	current_freq_ofs = ofs;
}




long TSI4431Class::GetCannelFreq (uint8_t cnl)
{
long fr = c_freq_chann_width_Mhz * cnl;
fr += c_freq_start_band_Mhz;
return fr;
}



void TSI4431Class::SetCannel (uint8_t cnl)
{
long fr = GetCannelFreq (cnl);
SetFreq (fr);
}

	//rfm23_write(0x3A, txHeader3);
	//rfm23_write(0x3B, txHeader2);
	//rfm23_write(0x3E, txPacketLength);
	//rfm23_burst(true, 0x7F, txFirstQueuedPkt->data, txPacketLength);
	//rfm23_tx_mode();

void TSI4431Class::Transmit (const S_RAWRADIO_CAPSUL_T &dat, uint8_t len)
{
	//u8 Status1, Status2;
	mux_tx_mode ();
	WriteRegister (0x07, 0x03);		// 0x01
	
	//WriteRegister(0x3A, 10);
	//WriteRegister(0x3B, 10);
	//WriteRegister(0x3C, 10);
	//WriteRegister(0x3D, 10);
	WriteRegister(0x3E, len);
	//WriteRegister (0x3E, len);
	u8 *DataSrc = (u8*)dat.raw;
	// data to fifo
	rfm23_burst(true, 0x7F, DataSrc, len);

	TxMode ();
	EZ_WAIT_NIRQ ();
			
	IdleMode ();
}



void TSI4431Class::HeaderSet (bool val, S_SI4431HDR_T *fltr)
{
f_header_en = val;
if (val)
	{
	if (fltr)
		{
		current_header = *fltr;
		}
	else
		{
		current_header.hform.hdr32 = 0;
		}
	}
}



bool TSI4431Class::CheckReceiver ()
{
	bool rv = false;
	return rv;
}



uint8_t TSI4431Class::Read (S_RAWRADIO_CAPSUL_T *dst)
{
	return Read ((uint8_t*)dst, sizeof(S_RAWRADIO_CAPSUL_T));
}



uint8_t TSI4431Class::Read (uint8_t *dst, uint8_t maxbufsize)
{
	uint8_t length = ReadRegister (0x4B);
	uint8_t rv = 0;
	bool f_need_clr_fifo = false;
	if (length > maxbufsize) 
		{
		length = maxbufsize;
		f_need_clr_fifo = true;
		}
	rv = length;
	rfm23_burst(false, 0x7F, dst, length);
	/*
	while (length)
		{
		dat = ReadRegister(0x7F);
		*dst++ = dat;						
		length--;
		}
	*/
		
	if (f_need_clr_fifo) FifoReset ();
		
	return rv;
}



bool TSI4431Class::FastScanRSSI (char *dstval)
{
	bool rv = false;
	return rv;
}



void TSI4431Class::RxOn (bool val)
{
	if (val)
		{
		RxMode ();
		}
	else
		{
		IdleMode ();
		}
}



void TSI4431Class::TxMode ()
{
	u8 Status1, Status2;
	// reset frequency offset set by AFC
	WriteRegister(0x73, 0x00);
	WriteRegister(0x74, 0x00);
	
	WriteRegister(0x05, 0x04);													//write 0x04 to the Interrupt Enable 1 register	
	WriteRegister(0x06, 0x00);													//write 0x00 to the Interrupt Enable 2 register	
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	EZ_RSTATUS (Status1, Status2);
	// enter TX mode, with PLL on (after TX mode, it will enter TUNE mode)
	WriteRegister(0x07, 0x0B); // 0x09 if disabled, SYNC packet is not sent, but bytes are sent in next DATA packet
	/*
	//Disable all other interrupts and enable the packet sent interrupt only.
	//This will be used for indicating the successfull packet transmission for the MCU
	WriteRegister(0x05, 0x04);													//write 0x04 to the Interrupt Enable 1 register	
	WriteRegister(0x06, 0x00);													//write 0x00 to the Interrupt Enable 2 register	
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	EZ_RSTATUS (Status1, Status2);
	//The radio forms the packet and send it automatically.
	WriteRegister(0x07, 0x09);													//write 0x09 to the Operating Function Control 1 register
	*/

	//enable the packet sent interupt only
	//WriteRegister(0x05, 0x04);													//write 0x04 to the Interrupt Enable 1 register		
	//read interrupt status to clear the interrupt flags
	//EZ_RSTATUS (Status1, Status2);
}



/*
void TSI4431Class::RxMode ()
{
		WriteRegister (0x07, 0x05);
		// a) 'ipkval'
		// b) CRC:  'icrcerror' 
		WriteRegister (0x05, 0x03);
		WriteRegister (0x06, 0x00);
}
*/





void TSI4431Class::RxMode ()
{
	u8 Status1, Status2;
	mux_rx_mode ();
	// enter Idle Mode
	WriteRegister(0x07, 0x03);		// 0x01
	
	WriteRegister (0x05, 0x03);
	WriteRegister (0x06, 0x00);
	EZ_RSTATUS (Status1, Status2);
	
	// reset RX FIFO
	WriteRegister(0x08, 0x22);
	WriteRegister(0x08, 0x20);
	// enter RX mode
	WriteRegister(0x07, 0x07);		// 0x05
	/*
	u8 Status1, Status2;
	// enter Idle Mode
	mux_rx_mode ();
	WriteRegister(0x07, 0x01);
	// reset RX FIFO
	FifoReset ();

		WriteRegister (0x05, 0x03);
		WriteRegister (0x06, 0x00);
		EZ_RSTATUS (Status1, Status2);
		WriteRegister (0x07, 0x05);
	*/
}



void TSI4431Class::IdleMode ()
{
	u8 Status1, Status2;
	mux_idle_mode ();
	//WriteRegister (0x05, 0x00);	// isr mode
	//WriteRegister (0x06, 0x00);	// isr mode
	WriteRegister(0x07, 0x01);
	EZ_RSTATUS (Status1, Status2);
}
	












