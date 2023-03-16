#include "gps_class.h"
#include "main.h"
#include <math.h>

TUSART_IFACE* TUSART_IFACE::lFS_PUSART[EUSARTINDX_MAX] = {0,0,0};

const char *ltxt_gngga = "GNGGA";
const char *ltxt_gnss = "GNGNS";
const char *ltxt_gnrmc = "GNRMC";
const char *ltxt_gngsa = "GNGSA";

uint8_t TCANARDGPS::cur_sat_amount = 0;
uint8_t TCANARDGPS::gpsDmode = 0;
volatile utimer_t TCANARDGPS::Timer_RTK_implement = 0;
char TCANARDGPS::linebuf[C_GPSBUF_SIZE];
uint32_t TCANARDGPS::push_ix = 0;


/*
static void wgsllh2ecef (double lat, double lon, double alt , ECEF_POINT &point)
{
	lat *= D2R;
	lon *= D2R;
  double d = WGS84_E * sin(lat);
  double N = WGS84_A / sqrt(1.0 - d * d);

  point.x = (N + alt) * cos(lat) * cos(lon);
  point.y = (N + alt) * cos(lat) * sin(lon);
  //point.z = (((1 - WGS84_E * WGS84_E) * N + alt) * lat);

  point.z = ((1 - WGS84_E * WGS84_E) * N + alt) * sin(lat);
}



static void wgsecef2llh(ECEF_POINT &ecef, WGS84POINT &llh)
{
  const double p = sqrt(ecef.x * ecef.x + ecef.y * ecef.y);

  if (p != 0) {
    llh.longitude = atan2(ecef.y, ecef.x);
  } else {
    llh.longitude = 0;
  }


  if (p < WGS84_A * 1e-16) {
    llh.latitude = copysign(M_PI_2, ecef.z);
    llh.altitude = fabs(ecef.z) - WGS84_B;

    llh.latitude *= R2D;
    llh.longitude *= R2D;
    return;
  }

  const double P = p / WGS84_A;
  const double e_c = sqrt(1. - WGS84_E * WGS84_E);
  const double Z = fabs(ecef.z) * e_c / WGS84_A;

  double S = Z;
  double C = e_c * P;


  double prev_C = -1;
  double prev_S = -1;

  double A_n, B_n, D_n, F_n;


  for (int i = 0; i < 10; i++) {

    A_n = sqrt(S * S + C * C);
    D_n = Z * A_n * A_n * A_n + WGS84_E * WGS84_E * S * S * S;
    F_n = P * A_n * A_n * A_n - WGS84_E * WGS84_E * C * C * C;
    B_n = 1.5 * WGS84_E * S * C * C * (A_n * (P * S - Z * C) - WGS84_E * S * C);


    S = D_n * F_n - B_n * S;
    C = F_n * F_n - B_n * C;



    if (S > C) {
      C = C / S;
      S = 1;
    } else {
      S = S / C;
      C = 1;
    }

    if (fabs(S - prev_S) < 1e-16 && fabs(C - prev_C) < 1e-16) {
      break;
    }
    prev_S = S;
    prev_C = C;
  }

  A_n = sqrt(S * S + C * C);
  llh.latitude = copysign(1.0, ecef.z) * atan(S / (e_c * C));
  llh.altitude = (p * e_c * C + fabs(ecef.z) * S - WGS84_A * e_c * A_n) / sqrt(e_c * e_c * C * C + S * S);


  llh.latitude *= R2D;
  llh.longitude *= R2D;
}
*/


/*
void lla2ecef (long double lat, long double lon, long double alt, ECEF_POINT &point)
{
    // WGS84 ellipsoid constants:
    const long double a = 6378137;            // (m) at the equator
    const long double e = 0.081819190842622;  // eccentricity

    // intermediate calculation
    // (prime vertical radius of curvature)
    long double N = a / sqrt(1 - (e * e) * (sin(lat) * sin(lat)));

    // results:
    point.x = (N + alt) * cos(lat) * cos(lon);
    point.y = (N + alt) * cos(lat) * sin(lon);
    point.z = ((1 - (e * e)) * N + alt) * sin(lat);
}



void ecef2lla_helper(long double x, long double y, long double z, WGS84POINT &coords)
{

    // WGS84 ellipsoid constants:
    const long double a = 6378137;            // (m) at the equator
    const long double e = 0.081819190842622;  // eccentricity

    // calculations:
    long double b   = sqrt( (a * a) * (1 - e * e ));
    long double ep  = sqrt( (a * a - b * b ) / (b * b));
    long double p   = sqrt( x * x + y * y );
    long double th  = atan2(a * z, b * p);
    long double lon = atan2(y, x);
    long double lat = atan2( (z + ep * ep * b * sin(th) * sin(th) * sin(th)), (p - e * e * a * cos(th) * cos(th) * cos(th)) );
    long double N   = a / sqrt( 1 - e * e * sin(lat) * sin(lat));
    long double alt = p / cos(lat) - N;

    // return lon in range [0,2*pi)
    lon = lon - (M_PI * 2) * floor( lon / (M_PI * 2) );

    coords.latitude = lat;
    coords.longitude = lon;
    coords.altitude = alt;
}
*/


static const unsigned long crc24q[256] = {
    0x00000000, 0x01864CFB, 0x028AD50D, 0x030C99F6,
    0x0493E6E1, 0x0515AA1A, 0x061933EC, 0x079F7F17,
    0x08A18139, 0x0927CDC2, 0x0A2B5434, 0x0BAD18CF,
    0x0C3267D8, 0x0DB42B23, 0x0EB8B2D5, 0x0F3EFE2E,
    0x10C54E89, 0x11430272, 0x124F9B84, 0x13C9D77F,
    0x1456A868, 0x15D0E493, 0x16DC7D65, 0x175A319E,
    0x1864CFB0, 0x19E2834B, 0x1AEE1ABD, 0x1B685646,
    0x1CF72951, 0x1D7165AA, 0x1E7DFC5C, 0x1FFBB0A7,
    0x200CD1E9, 0x218A9D12, 0x228604E4, 0x2300481F,
    0x249F3708, 0x25197BF3, 0x2615E205, 0x2793AEFE,
    0x28AD50D0, 0x292B1C2B, 0x2A2785DD, 0x2BA1C926,
    0x2C3EB631, 0x2DB8FACA, 0x2EB4633C, 0x2F322FC7,
    0x30C99F60, 0x314FD39B, 0x32434A6D, 0x33C50696,
    0x345A7981, 0x35DC357A, 0x36D0AC8C, 0x3756E077,
    0x38681E59, 0x39EE52A2, 0x3AE2CB54, 0x3B6487AF,
    0x3CFBF8B8, 0x3D7DB443, 0x3E712DB5, 0x3FF7614E,
    0x4019A3D2, 0x419FEF29, 0x429376DF, 0x43153A24,
    0x448A4533, 0x450C09C8, 0x4600903E, 0x4786DCC5,
    0x48B822EB, 0x493E6E10, 0x4A32F7E6, 0x4BB4BB1D,
    0x4C2BC40A, 0x4DAD88F1, 0x4EA11107, 0x4F275DFC,
    0x50DCED5B, 0x515AA1A0, 0x52563856, 0x53D074AD,
    0x544F0BBA, 0x55C94741, 0x56C5DEB7, 0x5743924C,
    0x587D6C62, 0x59FB2099, 0x5AF7B96F, 0x5B71F594,
    0x5CEE8A83, 0x5D68C678, 0x5E645F8E, 0x5FE21375,
    0x6015723B, 0x61933EC0, 0x629FA736, 0x6319EBCD,
    0x648694DA, 0x6500D821, 0x660C41D7, 0x678A0D2C,
    0x68B4F302, 0x6932BFF9, 0x6A3E260F, 0x6BB86AF4,
    0x6C2715E3, 0x6DA15918, 0x6EADC0EE, 0x6F2B8C15,
    0x70D03CB2, 0x71567049, 0x725AE9BF, 0x73DCA544,
    0x7443DA53, 0x75C596A8, 0x76C90F5E, 0x774F43A5,
    0x7871BD8B, 0x79F7F170, 0x7AFB6886, 0x7B7D247D,
    0x7CE25B6A, 0x7D641791, 0x7E688E67, 0x7FEEC29C,
    0x803347A4, 0x81B50B5F, 0x82B992A9, 0x833FDE52,
    0x84A0A145, 0x8526EDBE, 0x862A7448, 0x87AC38B3,
    0x8892C69D, 0x89148A66, 0x8A181390, 0x8B9E5F6B,
    0x8C01207C, 0x8D876C87, 0x8E8BF571, 0x8F0DB98A,
    0x90F6092D, 0x917045D6, 0x927CDC20, 0x93FA90DB,
    0x9465EFCC, 0x95E3A337, 0x96EF3AC1, 0x9769763A,
    0x98578814, 0x99D1C4EF, 0x9ADD5D19, 0x9B5B11E2,
    0x9CC46EF5, 0x9D42220E, 0x9E4EBBF8, 0x9FC8F703,
    0xA03F964D, 0xA1B9DAB6, 0xA2B54340, 0xA3330FBB,
    0xA4AC70AC, 0xA52A3C57, 0xA626A5A1, 0xA7A0E95A,
    0xA89E1774, 0xA9185B8F, 0xAA14C279, 0xAB928E82,
    0xAC0DF195, 0xAD8BBD6E, 0xAE872498, 0xAF016863,
    0xB0FAD8C4, 0xB17C943F, 0xB2700DC9, 0xB3F64132,
    0xB4693E25, 0xB5EF72DE, 0xB6E3EB28, 0xB765A7D3,
    0xB85B59FD, 0xB9DD1506, 0xBAD18CF0, 0xBB57C00B,
    0xBCC8BF1C, 0xBD4EF3E7, 0xBE426A11, 0xBFC426EA,
    0xC02AE476, 0xC1ACA88D, 0xC2A0317B, 0xC3267D80,
    0xC4B90297, 0xC53F4E6C, 0xC633D79A, 0xC7B59B61,
    0xC88B654F, 0xC90D29B4, 0xCA01B042, 0xCB87FCB9,
    0xCC1883AE, 0xCD9ECF55, 0xCE9256A3, 0xCF141A58,
    0xD0EFAAFF, 0xD169E604, 0xD2657FF2, 0xD3E33309,
    0xD47C4C1E, 0xD5FA00E5, 0xD6F69913, 0xD770D5E8,
    0xD84E2BC6, 0xD9C8673D, 0xDAC4FECB, 0xDB42B230,
    0xDCDDCD27, 0xDD5B81DC, 0xDE57182A, 0xDFD154D1,
    0xE026359F, 0xE1A07964, 0xE2ACE092, 0xE32AAC69,
    0xE4B5D37E, 0xE5339F85, 0xE63F0673, 0xE7B94A88,
    0xE887B4A6, 0xE901F85D, 0xEA0D61AB, 0xEB8B2D50,
    0xEC145247, 0xED921EBC, 0xEE9E874A, 0xEF18CBB1,
    0xF0E37B16, 0xF16537ED, 0xF269AE1B, 0xF3EFE2E0,
    0xF4709DF7, 0xF5F6D10C, 0xF6FA48FA, 0xF77C0401,
    0xF842FA2F, 0xF9C4B6D4, 0xFAC82F22, 0xFB4E63D9,
    0xFCD11CCE, 0xFD575035, 0xFE5BC9C3, 0xFFDD8538,
};




#ifdef __cplusplus
 extern "C" {
#endif 


void USART1_IRQHandler ()
{
	unsigned short flag_cr = TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->CR1;
	unsigned short flag_it = TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->ISR;
	
	unsigned char dat = TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->RDR;
  if (flag_it & (1<<5))
		{
    TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->ISR_DataRx (dat);
		}
  
	if (flag_cr & (1<<7))
		{
		if (flag_it & (1<<7))
			{
			unsigned char dat;
			if (TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->ISR_DataTx(&dat))
				{
				TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->TDR = dat;	
				}
			else
				{
				TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->CR1 &= ~(((unsigned short)1)<<7);
				TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->ICR |= 0xDF;
				}
			}
		}
TUSART_IFACE::lFS_PUSART[EUSARTINDX_GPS]->USARTPort->ICR |= 0x1F;
	
}


#ifdef __cplusplus
}
#endif


// TCANARDGPS::TCANARDGPS (uint16_t ff_rx_cnt) :C_GPS_LINEBUF_SIZE (ff_rx_cnt)
TCANARDGPS::TCANARDGPS ()
{
	//linebuf = new char [ff_rx_cnt];
	push_ix = 0;
	loc_cnt_nmea = 0;
	loc_cnt_messages = 0;
	f_new_gps_data = false;
	f_new_aux_data = false;
	//f_RTK_StationMode = false;
	
	DopsTimer = C_SPEEDSET_TIMEOUT;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&DopsTimer);
	ErrorTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&ErrorTimeout);

	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_UTC_value);
	
	UBXReqTimet_DOP = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&UBXReqTimet_DOP);
	UBXReqTimet_PVT = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&UBXReqTimet_PVT);
	UBXReqTimet_RATE = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&UBXReqTimet_RATE);
	UBXReqTimet_RTK = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&UBXReqTimet_RTK);
	Timer_RTK_implement = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_RTK_implement);

	memset (&GPSAccData, 0, sizeof(GPSAccData));
	speed_search = EUSARTSPD_460800;
	
	
	cur_sat_amount = 0;
	gpsDmode = 0;
	SWVal = EGPSSW_SPEED_DETECT_A;
	//cur_peack_rx_buf = 0;
}



bool TCANARDGPS::CalculateUBX_CRC (unsigned char *lSrc, unsigned short sz, TUBXCRC *lDstCrc)
{
	bool rv = false;
	unsigned char CK_A = 0;
	unsigned char CK_B = 0;
	unsigned char *lEnd = (unsigned char*)linebuf + C_GPSBUF_SIZE - 1;
	if (lSrc)
		{
		while (sz && lEnd >= lSrc)
			{
			CK_A = CK_A + lSrc[0];
			CK_B = CK_B + CK_A;
			lSrc++;
			sz--;
			}
		if (lEnd >= lSrc) rv = true;
		}
		
	if (lDstCrc)
		{
		lDstCrc->CK_A = CK_A;
		lDstCrc->CK_B = CK_B;
		}
return rv;		
}



void TCANARDGPS::WriteUBX_CRC (HDRUBX *ldst)
{
	if (ldst)
		{
		unsigned short cur_sz = sizeof(HDRUBX) - 2;		// 2 байта преамбулы
		cur_sz += ldst->Len;
		TUBXCRC *lDCrc = (TUBXCRC*)(((char*)ldst) + sizeof(HDRUBX) + ldst->Len);
		CalculateUBX_CRC ((unsigned char *)&ldst->Class, cur_sz, lDCrc);
		}
}



void TCANARDGPS::WriteUBXSignature (HDRUBX *ldst)
{
	ldst->Preamble_A = CUBXPREX_A;
	ldst->Preamble_B = CUBXPREX_B;
}



unsigned short TCANARDGPS::GetUBXFrameSize (HDRUBX *lframe)
{
	unsigned short rv = 0;
	if (lframe) rv = sizeof(HDRUBX) + sizeof(TUBXCRC) + lframe->Len;
	return rv;
}


// Version = 0
void TCANARDGPS::SendUBX_CFGVAL_set (void *InArray, unsigned long sizes, EUBXMEMTYPE lay)
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_CFG;
lFrame->Id = EUBXCFG_VALSET;
lFrame->Len = sizes;

WriteUBX_CRC (lFrame);	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}




void TCANARDGPS::SendUBX_CFGVAL_get (void *InArray, unsigned long sizes, EUBXMEMTYPE lay)
{
}



void TCANARDGPS::SendUBX_CFGVAL_del (void *InArray, unsigned long sizes, EUBXMEMTYPE lay)
{	
}





void TCANARDGPS::SendUBX_CFG_ROVER ()
{
UBXCGFRTKROVER *lFrame = (UBXCGFRTKROVER*)buf_tx;
WriteUBXSignature (&lFrame->Base.Hdr);
lFrame->Base.Hdr.Class = EUBXCLASS_CFG;
lFrame->Base.Hdr.Id = EUBXCFG_VALSET;
lFrame->Base.Hdr.Len = sizeof(UBXCGFRTKROVER) - sizeof(HDRUBX);
lFrame->Base.layers = 1;			// only ram
lFrame->Base.version = 0;			// version 0
unsigned char indx = 0;
while (indx < C_ROVERMESSAGE_AMOUNT)
	{
	lFrame->rover_mess[indx].key_id = RTKROVERENABLEARRAY[indx];
	lFrame->rover_mess[indx].value = 1;
	indx++;
	}
	
lFrame->rate_meas.key_id = C_CFG_RATE_MEAS;
lFrame->rate_meas.value = TUAVPARAMS::GetFixRateMs ();	// 200 ms
	
//lFrame->rtk_mod.key_id = C_CFG_NAVHPG_DGNSSMODE;
//lFrame->rtk_mod.value = 3;

	
WriteUBX_CRC (&lFrame->Base.Hdr);	
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Base.Hdr));
}



void TCANARDGPS::SendUBX_SetDGNSS (ERTKSUBMODE modd)
{
UBXCFGDGNSS *lFrame = (UBXCFGDGNSS*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_DGNSS;
lFrame->Hdr.Len = sizeof(UBXCFGDGNSS) - sizeof(HDRUBX);
lFrame->dgnssMode = modd;
WriteUBX_CRC (&lFrame->Hdr);	
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));	
}



void TCANARDGPS::SendUBX_NAV_DOP_req ()
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_NAV;
lFrame->Id = EUBXNAV_DOP;
lFrame->Len = 0;
WriteUBX_CRC (lFrame);
	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}



void TCANARDGPS::SendUBX_NAV_PVT_req ()
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_NAV;
lFrame->Id = EUBXNAV_PVT;
lFrame->Len = 0;
WriteUBX_CRC (lFrame);
	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}



void TCANARDGPS::SendUBX_CFG_RATE_req (unsigned short rate_ms)
{
UBXSETRATE *lFrame = (UBXSETRATE*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_RATE;
lFrame->Hdr.Len = sizeof(UBXSETRATE) - sizeof(HDRUBX);
lFrame->measRate = rate_ms;
lFrame->navRate = 1;	 	// 2
lFrame->timeRef = 0;		// utc
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}


void TCANARDGPS::SendUBX_SetSpeed (unsigned long spdset)
{
UBXSETSPEED *lFrame = (UBXSETSPEED*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_PRT;
lFrame->Hdr.Len = sizeof(UBXSETSPEED) - sizeof(HDRUBX);
lFrame->baudRate = spdset;
lFrame->portID = 1;					// usart 1
lFrame->mode = 0x8C0;				// 8 bit, no parity, 1 stop
#ifdef ZEDRTK_MODULE
	lFrame->inProtoMask = 3 | 32;			// nmea + ubx + rtcm3
	lFrame->outProtoMask = 3 | 32;		// nmea + ubx	+ rtcm3
#else
	lFrame->inProtoMask = 3;		// nmea + ubx
	lFrame->outProtoMask = 3;		// nmea + ubx	
#endif
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}



void TCANARDGPS::SendUBX_SetPMS ()
{
UBXSETPMS *lFrame = (UBXSETPMS*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_PMS;
lFrame->Hdr.Len = sizeof(UBXSETPMS) - sizeof(HDRUBX);
lFrame->version = 0;
lFrame->powerSetupValue = 4;			// 5 = 4 гц
lFrame->period = 0;
lFrame->onTime = 0;
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}



void TCANARDGPS::NVIC_USART_ENABLE (bool stat)
{
	if (stat)
		{
		NVIC_EnableIRQ (USART1_IRQn);
		}
	else
		{
		NVIC_DisableIRQ (USART1_IRQn);
		}
}



void TCANARDGPS::DeInit ()
{
NVIC_USART_ENABLE (false);
}



void TCANARDGPS::Init (unsigned long speed_set)	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USARTPort = C_USART_GPS_PORT;
	lFS_PUSART[EUSARTINDX_GPS] = (TUSART_IFACE*)this;
	
	if (!speed_set) 
		{
		if (speed_search >= EUSARTSPD_MAX) speed_search = EUSARTSPD_9600;
		speed_set = C_USARTSPEED_INDX[speed_search];
		}
	
	__HAL_RCC_USART1_CLK_ENABLE ();	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	tx_size = 0;
	USARTPort->CR1 &= ~(0xA0);		// USART_IT_RXNE & USART_IT_TXE

	f_tx_status = false;
	
	gps_raw_reset_pin (true);				// пасивное состояние RESET_N для GPS модуля = 1
	GPIO_InitStructure.Pin = C_RESETGPS_N_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;// GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	//GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	gps_raw_reset_pin (true);				// пасивное состояние RESET_N для GPS модуля = 1
	
	
	// RX,TX pins
	GPIO_InitStructure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	
  huart1.Instance = USART1;
  huart1.Init.BaudRate = speed_set;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_8;//UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_MSBFIRST_INIT;//UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
		{
			Error_Handler();
		}
	
	NVIC_USART_ENABLE (true);				// разрешить прерывания
	USART_ITConfig_RX (true);	
  USART_Cmd (USARTPort, ENABLE);
}



void TCANARDGPS::gps_hardware_reset ()
{
	//SWVal = EGPSSW_RESET;		- отключен
}



void TCANARDGPS::gps_raw_reset_pin (bool val)
{
	if(val)
		{
    GPIOB->BSRR = (uint32_t)C_RESETGPS_N_PIN;
		}
  else
		{
    GPIOB->BRR = (uint32_t)C_RESETGPS_N_PIN;
		}
}



void TCANARDGPS::ISR_DataRx (unsigned char dat)
{
if (push_ix < C_GPSBUF_SIZE) linebuf[push_ix++] = dat;
}



// устанавливает следующюю скорость и генерирует для ublox - команду перехода на приоритетную скорость работы 460800
void TCANARDGPS::NextSpeedSearch ()
{
char cspd = speed_search; cspd++;
loc_cnt_nmea = 0;
loc_cnt_messages = 0;
if (cspd >= EUSARTSPD_MAX) cspd = EUSARTSPD_9600; 
speed_search = (EUSARTSPD)cspd;
Init (C_USARTSPEED_INDX[speed_search]);		
UBX_SpeedSet_460800 ();
}



void TCANARDGPS::UBX_SpeedSet_460800 ()
{
SendUBX_SetSpeed (C_NEED_WORK_SPEED);
}



// 
void TCANARDGPS::ToDetectUsartSpeed ()
{
UBXReqTimet_RTK = 0;			// обнуления таймера отправки команды смены rate measure
UBXReqTimet_RATE = 0;
	SWVal = EGPSSW_SPEED_DETECT_A;
	NextSpeedSearch ();
	DopsTimer = C_SPEEDSET_TIMEOUT;
}



static unsigned char HexCharToBin (unsigned char datas)
{
unsigned char rv=0;
while (1)
        {
        if (datas>=97 && datas<=102)
                {
                rv=10+(datas-97);
                break;
                }
        if (datas>=65 && datas<=70)
                {
                rv=10+(datas-65);
                break;
                }
        if (datas>=48 && datas<=57)
                {
                rv=datas-48;
                break;
                }
        break;
        }
return rv;
}



static unsigned char CHex2Val (unsigned char *lpRams)
{
unsigned char rv = (HexCharToBin(lpRams[0]) << 4);
rv = rv | HexCharToBin(lpRams[1]);
return rv;
}



char *TCANARDGPS::FindFistDelimitter (char *lInpInBuf, uint32_t size, char delimm)
{
char *lpRv = 0;

char *lEnd = (char*)linebuf + C_GPSBUF_SIZE;
char  dat;
while ((lEnd > lInpInBuf) && size)
    {
    dat = lInpInBuf[0];
    if (!dat) break;
    if (dat == delimm)
        {
        lpRv = lInpInBuf;
        break;
        }
    lInpInBuf++;
    size--;
    }

return lpRv;
}




bool TCANARDGPS::check_nmea_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t last_psh_ix)
{
	bool rv = false;
	// находим разделители MNEA сообщений
	if (last_psh_ix)
			{
			char *lStart = FindFistDelimitter (linebuf, last_psh_ix, '$');

			if (lStart)
					{
					uint32_t fsize = lStart - linebuf;
					if (last_psh_ix >= fsize)
							{
							char *lStop = FindFistDelimitter (lStart, last_psh_ix - fsize , '*'); 
							if (lStop)
									{
									unsigned short full_mess_size = lStop - lStart + 3;  // + 1 + 2byte crc
									if ((lStop + 2) <= &linebuf[last_psh_ix])  // crc in bufer ?
											{
											unsigned char CRCX = 0;
											unsigned char *lCur = (unsigned char *)lStart + 1;
											unsigned short crcsz = lStop - lStart - 1;// full_mess_size - 4;
											// считаем контрольную сумму NMEA
											while (crcsz)
													{
													CRCX = CRCX ^ lCur[0];
													lCur++;
													crcsz--;
													}
											unsigned char dat = CHex2Val ((unsigned char *)lStop + 1);
											if (dat == CRCX)
													{
													mess_size = full_mess_size;
													ix_start = lStart - linebuf;
													rv = true;
													}
											}
									}
							}
					}
			}
return rv;
}



bool TCANARDGPS::check_ubx_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t last_psh_ix)
{
bool rv = false;
uint32_t ix = 0, datasize;
HDRUBX *lUbx;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lUbx = (HDRUBX*)&linebuf[ix];
    if (datasize > (sizeof(HDRUBX) + sizeof(TUBXCRC)))
        {
        if (lUbx->Preamble_A == CUBXPREX_A && lUbx->Preamble_B == CUBXPREX_B)
            {
            unsigned short full_len = sizeof(HDRUBX) + lUbx->Len + sizeof(TUBXCRC);   // + 2 byte crc
            if (datasize >= full_len)
                {
                // проверка контрольной суммы
                TUBXCRC ubx_Crc;
                CalculateUBX_CRC ((uint8_t*)&lUbx->Class, full_len - 4, &ubx_Crc);  // - 2 bytes preamble, -2 bytes crc
                TUBXCRC *lfnd_Crc = (TUBXCRC*)(((uint8_t*)lUbx) + sizeof(HDRUBX) + lUbx->Len);
                if (lfnd_Crc->CK_A == ubx_Crc.CK_A && lfnd_Crc->CK_B == ubx_Crc.CK_B)
                    {
                    ix_start = ix;
                    mess_size = full_len;
                    rv = true;
                    break;
                    }
                }
            else
                {
                // малый размер данных
                break;
                }
            }
        }
    else
        {
        // малый размер данных
        break;
        }
    ix++;
    }
return rv;
}



bool TCANARDGPS::check_rtcm_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t last_psh_ix)
{
bool rv = false;
HDRTRANPLAYRTCM3 *lframe;
uint32_t ix = 0, datasize;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lframe = (HDRTRANPLAYRTCM3*)&linebuf[ix];
    // предварительная проверка наличия целого пакета в буфере
    if (datasize > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // 3 byte header + 3 byte qcrc
        {
        if (lframe->Preamble_D3 == 0xD3)
            {
            unsigned short len_raw = SwapShort (lframe->len_raw);
            if (len_raw && len_raw < 1023)	// 6 reserved + 10 bits size
                {
                unsigned short len = sizeof(HDRTRANPLAYRTCM3) + len_raw;
                unsigned short full_len = len + C_RTCM3_CRC24_BSIZE;      // + 3 byte qcrc
                // финишная проверка наличия целого пакета в буфере
                if (datasize >= full_len)
                    {
                    unsigned long cur_crc = crc24q_hash ((unsigned char*)lframe, len);
                    if (compareLong24AndBuf (cur_crc, ((unsigned char*)lframe) + len))
                        {
                        mess_size = full_len; 
                        ix_start = ix;
                        rv = true;
                        break;
                        }
                    }
                else
                    {
                    // size is small
                    break;
                    }
                }
            }
        }
    else
        {
        // size is small
        break;
        }
    ix++;
    }
return rv;
}




void TCANARDGPS::cb_ubx_data (HDRUBX *lUbx, uint32_t size)
{

if (lUbx)
	{
	char *lpMessUBX = ((char*)lUbx) + sizeof (HDRUBX);				
	switch (lUbx->Class)
		{
		case EUBXCLASS_NAV:
			{
			switch (lUbx->Id)
				{
				case EUBXNAV_DOP:					// для ровера и станции
					{
					ParseUBXLine_DOP (lpMessUBX, lUbx->Len);
					break;
					}
				case EUBXNAV_PVT:					// для ровера и станции
					{
					ParseUBXLine_PVT (lpMessUBX, lUbx->Len);
					break;
					}
				default:
					{
					break;
					}
				}
			break;
			}
		case EUBXCLASS_MON:
			{
			break;
			}		
		case EUBXCLASS_ACK:
			{
			switch (lUbx->Id)
				{
				case EUBXACK_NACK:
					{
					ParseUBXLine_NACK (lpMessUBX, lUbx->Len);
					break;
					}
				case EUBXACK_ACK:
					{
					ParseUBXLine_ACK (lpMessUBX, lUbx->Len);
					break;
					}
				}
			break;
			}
		default:
			{
			break;
			}
		}
	}

}



bool TCANARDGPS::cb_nmea_data (uint8_t *inp, uint32_t size)
{
	bool rv = false;
	unsigned long FieldLen;			// размер поля в строке
	unsigned long TagMaxCount;	
	unsigned long size_line = 0;
	char *lFieldStr;
	if (inp)
		{
		GetTagStringDelimIndx ((char*)inp, 0xFF, ',', &lFieldStr, &TagMaxCount);		// определяет количество полей в nmea строке 
		FieldLen = GetTagStringDelimIndx ((char*)inp, 0, ',', &lFieldStr, 0);				// получение указателя на название поля и подсчет размера поля
		if (FieldLen >= 4)	// размерность поля типа отчета (по крайней мере больше или равно 4)
			{
			do 	{	// из необходимого нужны только GGA, RMC, GSA, GSV
					if (str_compare((char*)ltxt_gnss, lFieldStr, FieldLen))
						{
						ParseGNSSLine ((char*)inp, size_line, TagMaxCount);		// парсинг GNSS сообщения
						
						rv = true;
						break;
						}
					if (str_compare((char*)ltxt_gnrmc, lFieldStr, FieldLen))
						{
						ParseRMCLine ((char*)inp, size_line, TagMaxCount); 	// запустить парсинг RMC
						rv = true;
						break;
						}		
					if (str_compare((char*)ltxt_gngsa, lFieldStr, FieldLen))
						{
						ParseGSALine ((char*)inp, size_line, TagMaxCount); 	// запустить парсинг GSA
						rv = true;
						break;
						}
					} while (false);
			}
		}
return rv;
}



void TCANARDGPS::cb_rtcm3_data (uint8_t *inp, uint32_t size)
{
	
}



void TCANARDGPS::task_rx ()
{
	uint32_t ticks = SYSBIOS::GetTickCountLong ();

    uint32_t ix_start, find_size, loc_push_ix;
    bool f_detect;
    if (push_ix >= C_GPSBUF_SIZE) 
			{
			push_ix  = 0;
			return;
			}
	
		loc_push_ix = push_ix;
    while (loc_push_ix)
        {
				
				ix_start = 0, find_size = 0;
				f_detect = false;
				do  {
						if (check_nmea_linedata (ix_start, find_size, loc_push_ix))
								{
								if (find_size > 4)  // +$ +* +crc
										{
										uint32_t l_size = find_size - 4;
										uint8_t *adr = (uint8_t*)&linebuf[ix_start];
										adr += 1;       // + 1byte '$'
										if (cb_nmea_data (adr, l_size)) 
											{
											loc_cnt_nmea++;   // fps counter
											f_new_gps_data = true;
											//f_new_gps_data = true;
											}
										}

								f_detect = true;
								break;
								}
						if (check_ubx_linedata (ix_start, find_size, loc_push_ix))
								{
								if (find_size > (sizeof(HDRUBX) + sizeof(TUBXCRC)))
										{
										uint32_t l_size = find_size - sizeof(TUBXCRC);
										HDRUBX *adr = (HDRUBX*)&linebuf[ix_start];
										cb_ubx_data (adr, l_size);
										}

								f_detect = true;
								break;
								}
						if (check_rtcm_linedata (ix_start, find_size, loc_push_ix))
								{

								if (find_size > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // + 1=d3, +2=len, +3=bytes crc
										{
										uint32_t l_size = find_size - (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE);    // +3 bytes header + 3bytes crc
										uint8_t *adr = (uint8_t*)&linebuf[ix_start];
										adr += sizeof(HDRTRANPLAYRTCM3);   // +3 bytes header
										cb_rtcm3_data (adr, l_size);
										}

								f_detect = true;
								break;
								}
						} while (false);

          if (f_detect)
              {
              // проверяем нужно ли перемещение оставшихся данных в начало
              uint32_t last_ix = ix_start + find_size;
							loc_cnt_messages++;
              if (last_ix <= loc_push_ix)
                  {
									__disable_irq ();				// critical section
                  uint32_t tailsize = push_ix - last_ix;
                  CopyMemorySDC ((char*)&linebuf[last_ix], (char*)linebuf, tailsize);
                  push_ix = tailsize;
									loc_push_ix = push_ix;
									__enable_irq ();
                  }
              else
                {
                // непредвиденная ошибка
                push_ix = 0;
								break;
                }
              }
          else
            {
            break;
            }
        }
}




void TCANARDGPS::Task ()
{
	
	task_rx ();		// парсинг всех протоколов и сообщений
	
	if (GetTxStatus()) return;	

	switch (SWVal)
		{
		case EGPSSW_SPEED_DETECT_A:		// механизм обнаружения скорости
			{
			if (DopsTimer)
				{
				if (loc_cnt_messages >= 3)	// более 3-х сообщений принятых на текущей скорости
					{
					loc_cnt_messages = 0;
					if (speed_search == EUSARTSPD_460800)
						{
						// если пакеты обнаружены на нужной скорости - перейти на рабочий цикл
						SWVal = EGPSSW_WORK;
						ErrorTimeout = C_GPS_NODATA_TIMEOUT + 100;
						SendUBX_CFG_RATE_req (TUAVPARAMS::GetFixRateMs ());
						DopsTimer = C_MATTX_TIMEOUT;
						}
					else
						{
						// сообщения обнаружены, но скорость не оптимальная
						// установить оптимальную рабочую скорость
						speed_search = EUSARTSPD_460800;
						SendUBX_SetSpeed (C_USARTSPEED_INDX[speed_search]);
						DopsTimer = C_SPEEDSET_TIMEOUT;
						}
					}
				}
			else
				{
				// время поиска пакетов на текущей скорости истекло, переходим на следующюю скорость поиска
				NextSpeedSearch ();
				DopsTimer = C_SPEEDSET_TIMEOUT;							// таймаут ожидания приема пакетов на установленной скорости
				}
			break;
			}
		case EGPSSW_WORK:
			{
			if (loc_cnt_messages)	
				{
				loc_cnt_messages = 0;
				ErrorTimeout = C_GPS_NODATA_TIMEOUT;		// таймаут данных
				// если все нормально и данные приходят, значит можно отправлять комманды на этой скорости
				if (!DopsTimer)
					{
					switch (swUBXreq)
						{
						case ENEXTREQ_DOP:
							{
							if (!UBXReqTimet_DOP)
								{
								SendUBX_NAV_DOP_req ();	
								DopsTimer = C_MATTX_TIMEOUT;
								UBXReqTimet_DOP = C_PERIOD_DOP;
								}
							swUBXreq = ENEXTREQ_PVT;
							break;
							}
						case ENEXTREQ_PVT:
							{
							if (!UBXReqTimet_PVT)
								{
								SendUBX_NAV_PVT_req ();		// запрос на отсылку ubx_pvt
								DopsTimer = C_MATTX_TIMEOUT;
								UBXReqTimet_PVT = C_PERIOD_PVT;
								}
							swUBXreq = ENEXTREQ_RATE;
							break;
							}
						case ENEXTREQ_RATE:
							{
							if (!UBXReqTimet_RATE)
								{
								SendUBX_CFG_RATE_req (TUAVPARAMS::GetFixRateMs ());
								DopsTimer = C_MATTX_TIMEOUT;
								UBXReqTimet_RATE = C_PERIOD_RATE * 5;
								}
							swUBXreq = ENEXTREQ_RTKMODE;
							break;
							}
						case ENEXTREQ_RTKMODE:
							{
							if (!UBXReqTimet_RTK)
								{
								DopsTimer = C_MATTX_TIMEOUT;

								SendUBX_CFG_ROVER ();
									
								UBXReqTimet_RTK = C_PERIOD_RTK;
								}	
							swUBXreq = ENEXTREQ_DOP;
							break;
							}
						default:
							{
							swUBXreq = ENEXTREQ_DOP; 
							break;
							}
						}
					}
				}
			else
				{
				if (!ErrorTimeout) 
					{
					// данные не поступали в течении отведенного таймаута, 
					// переходим к поиску модуля на других скоростях
					ToDetectUsartSpeed ();
					}
				}
			break;
			}
		case EGPSSW_RESET:
			{
			gps_raw_reset_pin (false);			// активное состояние линии reset
			DopsTimer = 150; // 120
			SWVal = EGPSSW_RESET_WAIT;
			// без break
			}						
		case EGPSSW_RESET_WAIT:
			{
			if (!DopsTimer)
				{
				gps_raw_reset_pin (true);			// неактивное состояние reset
				SWVal = EGPSSW_SPEED_DETECT_A;
				}
			break;
			}
		default:
			{
			break;
			}
		}
}







// подсчитывает размер записи до перевода строки или нуля
unsigned long TCANARDGPS::GetLenGPS_NMEATag (char *lsrc)
{
unsigned long rv = 0;
if (lsrc)
	{
	char dat;
	while (true)
		{
		dat = lsrc[0];
		if (!dat || dat == 10 || dat == 13) break;
		rv++;
		lsrc++;
		}
	}
return rv;	
}



bool TCANARDGPS::compareLong24AndBuf (unsigned long curdata, unsigned char *lTxt)
{
bool rv = false;
if ((curdata & 0xFF) == lTxt[2] && ((curdata >> 8) & 0xFF) == lTxt[1] && ((curdata >> 16) & 0xFF) == lTxt[0]) rv = true;
return rv;	
}



bool TCANARDGPS::CheckRTCM_Format (HDRTRANPLAYRTCM3 *lframe, unsigned long *cut_sz)
{
bool rv = false;
if (lframe)
	{
	if (lframe->Preamble_D3 == 0xD3)		// magic preamble
		{
		unsigned short len_raw = SwapShort (lframe->len_raw);
		if (len_raw && len_raw < 1023)	// 6 reserved + 10 bits size
			{
			unsigned short len = sizeof(HDRTRANPLAYRTCM3) + len_raw;
			char *lEnd = linebuf + push_ix;
			if (lEnd > (((char*)lframe) + len + 3))		// + 24bit qcrc
				{
				unsigned long cur_crc = crc24q_hash ((unsigned char*)lframe, len);
				if (compareLong24AndBuf (cur_crc, ((unsigned char*)lframe) + len))
					{
					rv = true;
					if (cut_sz) *cut_sz = len + 3;
					}
				}
			}
		}
	}
return rv;
}
	


bool TCANARDGPS::CheckUBX_Format (HDRUBX *lframe, unsigned long *cut_sz)
{
	bool rv = false;
	if (lframe)
		{
		char *lEnd = linebuf + push_ix - 1;
		char *lEndFrame = (char*)lframe;
		if (lEnd > (lEndFrame + sizeof(HDRUBX)))
			{
			if (lframe->Preamble_A == CUBXPREX_A && lframe->Preamble_B == CUBXPREX_B)
				{	
				TUBXCRC cur_Crc;
				if (CalculateUBX_CRC ((unsigned char *)&lframe->Class, lframe->Len + sizeof(HDRUBX) - 2, &cur_Crc))
					{
					TUBXCRC *lfrm_crc = (TUBXCRC*)(((char*)lframe) + sizeof(HDRUBX) + lframe->Len);
					if (lfrm_crc->CK_A == cur_Crc.CK_A && lfrm_crc->CK_B == cur_Crc.CK_B) 
						{
						if (cut_sz) *cut_sz = sizeof(HDRUBX) + lframe->Len + sizeof(TUBXCRC);
						rv = true;
						}
					}
				}
			}
		}
	return rv;
}


/*
bool TCANARDGPS::CheckNMEA_Format (char *lTxt, unsigned long *cut_sz)
{
bool rv = false;
	// считаем контрольную сумму NMEA
	char *lStart = FindFistDelimitter (lTxt, '$');
	char *lStop = FindFistDelimitter (lTxt, '*');
	if (lStart && lStop && lStop > lStart)
		{
		unsigned short sz = lStop - lStart;
		if (sz >= 3)
			{
			unsigned char CRCX = 0;
			unsigned char *lCur = (unsigned char*)lStart + 1;
			unsigned short line_sz = sz + 3;		// crc и +1
			sz--;
			while (sz)
				{
				CRCX = CRCX ^ lCur[0];
				lCur++;
				sz--;
				}
			unsigned char dat = ConvertHex2Val ((unsigned char *)lStop + 1);
			if (dat == CRCX) 
				{
				if (cut_sz) *cut_sz = line_sz;
				rv = true;
				}
					
			}
		}
		
return rv;
}
*/


/*
char *TCANARDGPS::GetNextStringTag (unsigned long *cut_sz, EGPSMESSTYPE &typetag)
{
	char *lpRv = 0;
	typetag = EGPSMESSTYPE_NONE;
	if (push_ix)
		{
		lCurFindAdr += CurFindTagSize;
		char *lEnd = linebuf + push_ix - 6;	
		
		while (lEnd > lCurFindAdr)	
			{
			// еще находимся внутри принятого блока
			CurFindTagSize = 0;
			if (lCurFindAdr[0] == '$')
				{
				CurFindTagSize = GetLenGPS_NMEATag (lCurFindAdr);
				if (CurFindTagSize)
					{
					lpRv = lCurFindAdr;
					typetag = EGPSMESSTYPE_NMEA;
					break;
					}
				}
			else
				{
				if (CheckUBX_Format ((HDRUBX*)lCurFindAdr, &CurFindTagSize))
					{
					lpRv = lCurFindAdr;
					typetag = EGPSMESSTYPE_UBX;
					break;
					}
				else
					{
					if (CheckRTCM_Format ((HDRTRANPLAYRTCM3*)lCurFindAdr, &CurFindTagSize))
						{
						lpRv = lCurFindAdr;
						typetag = EGPSMESSTYPE_RTCM;
						break;
						}
					else
						{
						lCurFindAdr++;
						}
					}
				}
			}
		if (cut_sz && lpRv) *cut_sz = CurFindTagSize;
		}
	return lpRv;
}
*/


/*
unsigned short TCANARDGPS::Parse ()
{
	unsigned long size_line = 0;
	unsigned short FrameLineAmount = 0;
	EGPSMESSTYPE typline;
	unsigned long sz_rtcm = 0;
	static unsigned long max_sz_rtcm = 0;
	bool f_error = false;
	char *lTagLine = GetFistStringTag (&size_line, typline);
	
	while (lTagLine && !f_error)
		{
		switch (typline)
			{
			case EGPSMESSTYPE_UBX:
				{
				if (ParseUBX_OneLine (lTagLine, size_line))
					{
					}
				else
					{
					f_error = true;
					}
				break;
				}
			case EGPSMESSTYPE_NMEA:
				{

				break;
				}
			case EGPSMESSTYPE_RTCM:
				{

				break;
				}
			default:
				{
				break;
				}
			}
		if (f_error) break;
		FrameLineAmount++;
		lTagLine = GetNextStringTag (&size_line, typline);
		}
	
	if (sz_rtcm > max_sz_rtcm) max_sz_rtcm = sz_rtcm;
	return FrameLineAmount;
}
*/



//static unsigned long Test[11];


/*
bool TCANARDGPS::ParseRTCM3_1005Mess (unsigned char *ldata, unsigned long sizes, RTCM3_1005MESSAG &m_out)
{
bool rv = false;
if (ldata && sizes)
	{
	unsigned short Cofs = 0;
	m_out.MessageNumber = CreateValueFromBitMassive (ldata, 152, Cofs, 12);	Cofs += 12;
	m_out.ReferenceStationID = CreateValueFromBitMassive (ldata, 152, Cofs, 12);	Cofs += 12;
	m_out.ReservedforITRFRealizationYear = CreateValueFromBitMassive (ldata, 152, Cofs, 6);	Cofs += 6;
	m_out.GPSIndicator = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.GLONASSIndicator = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.ReservedforGalileoIndicator = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.ReferenceStationIndicator = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.AntennaReferencePointECEF_X = CreateValueFromBitMassive64out (ldata, 152, Cofs, 38);	Cofs += 38;
	m_out.SingleReceiverOscillatorIndicator = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.Reserved1 = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;	
	m_out.AntennaReferencePointECEF_Y = CreateValueFromBitMassive64out (ldata, 152, Cofs, 38);	Cofs += 38;
	m_out.Reserved2 = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.Reserved3 = CreateValueFromBitMassive (ldata, 152, Cofs, 1);	Cofs += 1;
	m_out.AntennaReferencePointECEF_Z = CreateValueFromBitMassive64out (ldata, 152, Cofs, 38);	//Cofs += 38;	
	rv = true;
	}
return rv;
}
*/




			/*
			WGS84POINT dest_llh;
			ECEF_POINT inp_ecef;
			inp_ecef.x = m_1005m.AntennaReferencePointECEF_X/10000;
			inp_ecef.y = m_1005m.AntennaReferencePointECEF_Y/10000;
			inp_ecef.z = m_1005m.AntennaReferencePointECEF_Z/10000;		
				
			// 42.78151669501    15.95570953483    -5894626.055583  + 0
			// 42.7815152772		 15.95572148453		 -5894625.881053	+ 11
			wgsecef2llh(inp_ecef, dest_llh);			
				
			if (rv)
				{

				WGS84POINT coords;
				ECEF_POINT point;
				wgsllh2ecef (46.46509717, 30.70189650, 28.7, point);	// x = 3784140.540025, y = 2247027.647603, z = 4601029.652859
					
				coords.altitude = coords.altitude;
				}
			*/

//static unsigned char datsss[200];





bool TCANARDGPS::ParseGNSSLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_GNSS_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;

		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_UTC, ',', &lFieldStr, 0);
		rv |= ParseFieldUTC (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LAT, ',', &lFieldStr, 0);
		rv |= ParseFieldLatitude (lFieldStr, LenField);
			
		// lat pole parse 
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LAT_NS, ',', &lFieldStr, 0);
		ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse 
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LONG, ',', &lFieldStr, 0);
		ParseFieldLongitude (lFieldStr, LenField);
			
		// long polus parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LON_EW, ',', &lFieldStr, 0);
		ParseFieldLongEW (lFieldStr, LenField);
		
		/* данные о режиме берем из PVT
		// quality parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_QUAL, ',', &lFieldStr, 0);
		ParseFieldPosMode (lFieldStr, LenField);	
		*/
		
		// satelite numbers parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_NBRSSATEL, ',', &lFieldStr, 0);
		ParseFieldNmbsSatelite (lFieldStr, LenField);
		
		
		// Altitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_ALTIT, ',', &lFieldStr, 0);
		ParseFieldAltitude (lFieldStr, LenField);
		
		// dif elipsoide parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_D_ELIPS, ',', &lFieldStr, 0);
		ParseFieldDifElipsHeight (lFieldStr, LenField);
		
		}
	return rv;
}



bool TCANARDGPS::ParseGGALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_GGA_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;

		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_UTC, ',', &lFieldStr, 0);
		rv |= ParseFieldUTC (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT, ',', &lFieldStr, 0);
		rv |= ParseFieldLatitude (lFieldStr, LenField);
			
		// lat pole parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT_NS, ',', &lFieldStr, 0);
		ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LONG, ',', &lFieldStr, 0);
		ParseFieldLongitude (lFieldStr, LenField);
			
		// long polus parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LON_EW, ',', &lFieldStr, 0);
		ParseFieldLongEW (lFieldStr, LenField);
		
		// quality parse
		//LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_QUAL, ',', &lFieldStr, 0);
		//ParseFieldQuality (lFieldStr, LenField);
		
		// satelite numbers parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_NBRSSATEL, ',', &lFieldStr, 0);
		ParseFieldNmbsSatelite (lFieldStr, LenField);
		
		// Altitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_ALTIT, ',', &lFieldStr, 0);
		ParseFieldAltitude (lFieldStr, LenField);
		
		// dif elipsoide parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_D_ELIPS, ',', &lFieldStr, 0);
		ParseFieldDifElipsHeight (lFieldStr, LenField);
		
		}
	return rv;
}



bool TCANARDGPS::ParseRMCLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_RMC_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTC, ',', &lFieldStr, 0);
		rv |= ParseFieldUTC (lFieldStr, LenField);
			
		// valid parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_VALID, ',', &lFieldStr, 0);
		ParseFieldValid (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT, ',', &lFieldStr, 0);
		rv |= ParseFieldLatitude (lFieldStr, LenField);
			
		// latit poluse parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT_NS, ',', &lFieldStr, 0);
		ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LONG, ',', &lFieldStr, 0);
		rv |= ParseFieldLongitude (lFieldStr, LenField);
		
		// longitude polus parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LON_EW, ',', &lFieldStr, 0);
		ParseFieldLongEW (lFieldStr, LenField);
		
		// speedov parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_SPEEDOV, ',', &lFieldStr, 0);
		ParseFieldSpeedOV (lFieldStr, LenField);
		
		// course parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_COURSE, ',', &lFieldStr, 0);
		ParseFieldCourse (lFieldStr, LenField);
		
		// UTD parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTD, ',', &lFieldStr, 0);
		ParseFieldUTD (lFieldStr, LenField);
		
		// magnet var parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGNETVAR, ',', &lFieldStr, 0);
		ParseFieldMagnetVar (lFieldStr, LenField);
		
		// EW parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGEW, ',', &lFieldStr, 0);
		ParseFieldMagnetEW (lFieldStr, LenField);
		}
	return rv;
}



bool TCANARDGPS::ParseGSALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
bool rv = false;
		if (amount_filed >= C_GSA_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// fix mod parse
		/*
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_FIXMOD, ',', &lFieldStr, 0);
		ParseFieldFixMode (lFieldStr, LenField);
		*/
			
		// pdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_PDOP, ',', &lFieldStr, 0);
		rv |= ParseFieldPDOP (lFieldStr, LenField);
			
		// hdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_HDOP, ',', &lFieldStr, 0);
		rv |= ParseFieldHDOP (lFieldStr, LenField);
			
		// hdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_VDOP, ',', &lFieldStr, 0);
		rv |= ParseFieldVDOP (lFieldStr, LenField);
			
		f_new_aux_data = rv;
		}
return rv;		
}



bool TCANARDGPS::ParseGSVLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
bool rv = false;
		if (amount_filed >= C_GSV_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// satelite view number parse
		LenField = GetTagStringDelimIndx (lTxt, EGSVINX_VISIBSAT, ',', &lFieldStr, 0);
		rv = ParseFieldSatView (lFieldStr, LenField);
		
		}
return rv;
}



bool TCANARDGPS::ParseFieldSatView(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >= 2)
	{
	unsigned long DataOut = 0;
	do  {
			// satelit view count
			if (!TxtToULong (lTxt, sizes, &DataOut)) break;
			if (DataOut <= 127)
				{
				GPSAccData.SatelitViewN = DataOut;
				rv = true;
				}
			} while (false);
	}
GPSAccData.f_SatViewN_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldPDOP(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.PDOP.value = outd;
		rv = true;
		}
	}
GPSAccData.PDOP.f_valid = rv;
return rv;	
}



bool TCANARDGPS::ParseFieldVDOP(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >=3)
	{
	float outd = 0;
	sizes = 3;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.VDOP.value = outd;
		rv = true;
		}
	}
GPSAccData.VDOP.f_valid = rv;
return rv;	
}



bool TCANARDGPS::ParseFieldHDOP (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.HDOP.value = outd;
		rv = true;
		}
	}
GPSAccData.HDOP.f_valid = rv;
return rv;	
}


/*
bool TCANARDGPS::ParseFieldFixMode(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		if (dat >= '0' && dat <= '3')
			{
			GPSAccData.Mode2D3D = dat;
			rv = true;
			}
		}
	}
GPSAccData.f_FixMode_valid = rv;
return rv;	
}
*/


bool TCANARDGPS::ParseFieldMagnetEW (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		switch (dat)
			{
			case 'W':  case 'E': 
				{
				GPSAccData.Mag_EW = dat;
				rv = true;
				break;
				}
			}
		}
	}
GPSAccData.f_MagnetVAR_EW_valid = rv;
return rv;	
}



bool TCANARDGPS::ParseFieldMagnetVar (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.MagVariation = outd;
		rv = true;
		}
	}
GPSAccData.f_MagnetVAR_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldCourse (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.TrueCourse = outd;
		rv = true;
		}
	}
GPSAccData.f_Course_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldUTD (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >= 6)
	{
	unsigned long DataOut = 0;
	do  {
			// день
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Day = DataOut;
		
			// месяц
			lTxt += 2;
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Month = DataOut;
		
			// год
			lTxt += 2;
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Year = DataOut;
		
			rv = true;
			} while (false);
	}
GPSAccData.f_UTD_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldSpeedOV (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.Speed = outd;
		rv = true;
		}
	}
GPSAccData.f_Speed_valid = rv;
return rv;
}
	


bool TCANARDGPS::ParseFieldValid (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		switch (dat)
			{
			case 'A': case 'V':
				{
				GPSAccData.ValidityStatus = dat;
				rv = true;
				break;
				}
			}
		}
	}
GPSAccData.f_ValidField_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldUTC (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes >= 6)
		{
		unsigned long DataOut = 0;
		float outd;
		do  {
				// часы
				if (!TxtToULong (lTxt, 2, &DataOut)) break;
				GPSAccData.UTC_time.Hour = DataOut;
			
				// минуты
				lTxt += 2;
				if (!TxtToULong (lTxt, 2, &DataOut)) break;
				GPSAccData.UTC_time.Minute = DataOut;
			
				// секунды могут быть float
				lTxt += 2;
				sizes -= 4;
				if (!TxtToFloat (&outd, lTxt, sizes)) break;
				GPSAccData.UTC_time.seconds = outd;
			
				rv = true;
				} while (false);
		}
	}
	
	if (rv)		// utc данные получены
		{
		// контролируем зависание gps модуля по заморозке utc значения
		if (utc_last_seconds_value != GPSAccData.UTC_time.seconds)
			{
			utc_last_seconds_value = GPSAccData.UTC_time.seconds;
			Timer_UTC_value = C_NMEA_UTC_TAG_TIMEOUT;
			}
		}
	else
		{
		// если utc данных нет, значит отключаем контроль заморозки utc времени
		Timer_UTC_value = C_NMEA_UTC_TAG_TIMEOUT;
		}
GPSAccData.f_UTC_valid = rv;
return rv;	
}





bool TCANARDGPS::ParseFieldLatNS (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		switch (dat)
			{
			case 'N':  case 'S': 
				{
				GPSAccData.Lat_NS = dat;
				rv = true;
				break;
				}
			}
		}
	}
GPSAccData.f_Lat_NS_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldLatitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes > 2)
	{
	float outd = 0;
	unsigned long Angle;
	// градусы широты
	do {
			if (!TxtToULong (lTxt, C_NMEASIZELAT_DEG, &Angle)) break;		// два символа
			sizes -= C_NMEASIZELAT_DEG; lTxt += C_NMEASIZELAT_DEG;
			// минуты широты
			if (!TxtToFloat (&outd, lTxt, sizes)) break;
			GPSAccData.Latitude = outd; GPSAccData.Latitude = (GPSAccData.Latitude / 60) + Angle;
			rv = true;	
			} while (false);
	}
GPSAccData.f_Lat_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldLongitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes > 5)
	{
	float outd = 0;
	unsigned long Angle;
	// градусы долготы
	do {
			if (!TxtToULong (lTxt, C_NMEASIZELONG_DEG, &Angle)) break;		// три символа
			sizes -= C_NMEASIZELONG_DEG; lTxt += C_NMEASIZELONG_DEG;
			// минуты долготы
			if (!TxtToFloat (&outd, lTxt, sizes)) break;
			GPSAccData.Longitute = outd; GPSAccData.Longitute = (GPSAccData.Longitute / 60) + Angle;
			rv = true;	
			} while (false);
	}
GPSAccData.f_Long_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldLongEW (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		switch (dat)
			{
			case 'E': case 'W': 
				{
				GPSAccData.Lon_EW = dat;
				rv = true;
				break;
				}
			}
		}
	}
GPSAccData.f_Long_EW_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldPosMode (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	unsigned char indx = 0;
	char dat;
	char cur_q = 0;
	while (indx < sizes)
		{
		dat = *lTxt;
		if (dat == 'D' ||  dat == 'A' || dat == 'F' || dat == 'R')
			{
			if (dat == 'F' || dat == 'R') Timer_RTK_implement = C_TIME_LED_RTK_STATUS;
			cur_q = 3;
			break;
			}
		if (dat == 'E') 
			{
			cur_q = 1;
			}
		lTxt++;
		indx++;
		}
	rv = true;
	GPSAccData.Mode2D3D = cur_q;
	}
else
	{
	GPSAccData.Mode2D3D = 0;
	}
GPSAccData.f_FixMode_valid = rv;
//GPSAccData.f_Quality_valid = rv;
return rv;	
}



/*
Fix Quality:
- 0 = Invalid
- 1 = GPS fix
- 2 = DGPS fix
*/
/*
bool TCANARDGPS::ParseFieldQuality (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0] - '0';
		if (dat <= 2)	// 0 - нет позиции, 1 - позиция SPS, 2 - позиция DGPS
			{
			if (dat == 2)
				{
				rv = rv;
				}
			GPSAccData.FixQuality = dat;
			rv = true;
			}
		}
	}
GPSAccData.f_Quality_valid = rv;
return rv;
}
*/


bool TCANARDGPS::ParseFieldNmbsSatelite (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes && sizes <= 3)
	{
	unsigned long DataOut = 0;
	if (TxtToULong (lTxt, sizes, &DataOut))
		{
		GPSAccData.SatelitAmount = DataOut;
		GPSAccData.SatelitViewN = DataOut;
		rv = true;
		}
	}
GPSAccData.f_SatelitNumb_valid = rv;
GPSAccData.f_SatViewN_valid = rv;
return rv;
}





bool TCANARDGPS::ParseFieldAltitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.Altitude = outd;
		rv = true;
		}
	}
GPSAccData.f_Altitude_valid = rv;
return rv;
}



bool TCANARDGPS::ParseFieldDifElipsHeight (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.DifElipsHeight = outd;
		rv = true;
		}
	}
GPSAccData.f_DifElipsHeight_valid = rv;
return rv;
}




bool TCANARDGPS::GetFixData (uavcan_equipment_gnss_Fix &datt)
{
	bool rv = check_fix_data ();
	if (rv)
		{
		memset (&datt, 0, sizeof(datt));
		// latitude
		if (GPSAccData.f_Lat_valid && GPSAccData.f_Lat_NS_valid)
			{
			datt.latitude_deg_1e8 = ((double)GPSAccData.Latitude) * 1E8;
			if (GPSAccData.Lat_NS == 'S') datt.latitude_deg_1e8 *= -1;
			}
		else
			{
			// если нет направления N S
			datt.latitude_deg_1e8 = 0;
			}
		// longitude
		if (GPSAccData.f_Long_valid && GPSAccData.f_Long_EW_valid)
			{
			datt.longitude_deg_1e8 = ((double)GPSAccData.Longitute) * 1E8;		
			if (GPSAccData.Lon_EW == 'W') datt.longitude_deg_1e8 *= -1;
			}
		else
			{
			// если нет направления E W
			datt.longitude_deg_1e8 = 0;
			}
		if (GPSAccData.f_SatelitNumb_valid) 
			{
			datt.sats_used = GPSAccData.SatelitAmount;
			}
		else
			{
			datt.sats_used = 0;	
			}
		cur_sat_amount = datt.sats_used;
		if (GPSAccData.f_UTC_valid && GPSAccData.f_UTD_valid) 
			{
			double dval = GPSAccData.UTC_time.Hour; dval *= 3600;
			dval += (((float)GPSAccData.UTC_time.Minute) * 60);
			dval += GPSAccData.UTC_time.seconds;
			unsigned long dayamount = GetUTCDaysToDate (2000 + GPSAccData.UTD_date.Year, GPSAccData.UTD_date.Month, GPSAccData.UTD_date.Day);
			dayamount *= 86400;
			datt.gnss_timestamp.usec = (dval + dayamount);// * 1000000;
				
			datt.gnss_time_standard = 2;		// utc
			}
		else
			{
			// если ошибка по времени
			datt.gnss_timestamp.usec = 0;
			datt.gnss_time_standard = 0;		// unknow
			}
		if (GPSAccData.PDOP.f_valid)
			{
			datt.pdop = GPSAccData.PDOP.value;
			}
		else
			{
			datt.pdop = C_DOP_NAN;	
			}
		if (GPSAccData.f_FixMode_valid && GPSAccData.f_SatelitNumb_valid)
			{
			datt.status = GPSAccData.Mode2D3D;
			if (GPSAccData.Mode2D3D == 3)
				{
				if (GPSAccData.SatelitAmount <= 3 || !GPSAccData.f_Altitude_valid ) datt.status = 2;		// 2d
				}
			}
		else
			{
			datt.status = 0;
			}
		gpsDmode = datt.status;
			
		if (GPSAccData.f_DifElipsHeight_valid && GPSAccData.f_Altitude_valid)	// елипсоидная высота высчитывается из высоты над морем
			{
			float elips_h = (GPSAccData.Altitude + GPSAccData.DifElipsHeight) * 1000;		// метры в милиметры
			datt.height_ellipsoid_mm = elips_h;
			}
		else
			{
			datt.height_ellipsoid_mm  = 0;
			}

		if (GPSAccData.f_Altitude_valid)
			{
			float dat = GPSAccData.Altitude * 1000;		// метры в милиметры
			datt.height_msl_mm = dat;
			}
		else
			{
			datt.height_msl_mm = 0;
			}

		if (GPSAccData.VELN.f_valid)
			{
			datt.ned_velocity[0] = GPSAccData.VELN.value_m_s;
			}
		else
			{
			datt.ned_velocity[0] = 0;
			}
		
		if (GPSAccData.VELE.f_valid)
			{
			datt.ned_velocity[1] = GPSAccData.VELE.value_m_s;
			}
		else
			{
			datt.ned_velocity[1] = 0;
			}

		if (GPSAccData.VELD.f_valid)
			{
			datt.ned_velocity[2] = GPSAccData.VELD.value_m_s;
			}
		else
			{
			datt.ned_velocity[2] = 0;
			}
			
		datt.num_leap_seconds = 27;
			
		rv = true;
		}
	//f_new_gps_data = false;
	return rv;
}



TCOVRPARAMS *TCANARDGPS::GetCovariances ()
{
	TCOVRPARAMS *rv = 0;
	if (check_fix_data ()) rv = &GPSAccData.COVARIANCES;
	return rv;
}


// готово, все поля заполнены
bool TCANARDGPS::GetAuxData (uavcan_equipment_gnss_Auxiliary &datt)
{
	bool rv = check_aux_data ();
	if (rv)
		{
		if (GPSAccData.f_SatelitNumb_valid) 
			{
			datt.sats_used = GPSAccData.SatelitAmount;
			}
		else
			{
			datt.sats_used = 0;
			}
		if (GPSAccData.f_SatViewN_valid)
			{
			datt.sats_visible = GPSAccData.SatelitViewN;	
			}
		else
			{
			datt.sats_visible = 0;	
			}
		// ----------- gdop
		if (GPSAccData.GDOP.f_valid)
			{
			datt.gdop = GPSAccData.GDOP.value;
			}
		else
			{
			datt.gdop = C_DOP_NAN;
			}
		// ----------- pdop
		if (GPSAccData.PDOP.f_valid)
			{
			datt.pdop = GPSAccData.PDOP.value;
			}
		else
			{
			datt.pdop = C_DOP_NAN;
			}
		// ----------- tdop
		if (GPSAccData.TDOP.f_valid)
			{
			datt.tdop = GPSAccData.TDOP.value;
			}
		else
			{
			datt.tdop = C_DOP_NAN;
			}
		// ----------- vdop
		if (GPSAccData.VDOP.f_valid)
			{
			datt.vdop = GPSAccData.VDOP.value;
			}
		else
			{
			datt.vdop = C_DOP_NAN;
			}
		// ----------- hdop
		if (GPSAccData.HDOP.f_valid)
			{
			datt.hdop = GPSAccData.HDOP.value;
			}
		else
			{
			datt.hdop = C_DOP_NAN;	
			}
		// ----------- ndop
		if (GPSAccData.NDOP.f_valid)
			{
			datt.ndop = GPSAccData.NDOP.value;
			}
		else
			{
			datt.ndop = C_DOP_NAN;
			}
		// ----------- edop
		if (GPSAccData.EDOP.f_valid)
			{
			datt.edop = GPSAccData.EDOP.value;
			}
		else
			{
			datt.edop = C_DOP_NAN;
			}
		rv = true;
		}
	return rv;
}



bool TCANARDGPS::check_fix_data ()
{
return f_new_gps_data;
}



bool TCANARDGPS::check_aux_data ()
{
return f_new_aux_data;
}



void TCANARDGPS::clr_fix_flag ()
{
 f_new_gps_data = false;
}



void TCANARDGPS::clr_aux_flag ()
{
 f_new_aux_data = false;
}



/*
bool TCANARDGPS::ParseUBXLine_NAVPOSLLH (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}


unsigned long debg_one[3];
unsigned long debg_zero[3];



bool TCANARDGPS::ParseUBXLine_NAVRELPOSNED (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXRELPOSNED))
		{
		FUBXRELPOSNED *lFr = (FUBXRELPOSNED*)lTxt;
		lFr->accD = lFr->accD;
		unsigned char indx = 0;
		unsigned char mask = 1;
		unsigned long *lpInc = 0;
		while (indx < 3)
			{
			if (lFr->flags & mask)
				{
				lpInc = &debg_one[indx];	
				}
			else
				{
				lpInc = &debg_zero[indx];	
				}
			lpInc[0]++;
			mask = mask << 1;
			indx++;
			}
		rv = true;
		}
return rv;	
}



bool TCANARDGPS::ParseUBXLine_NAVSTATUS (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}



bool TCANARDGPS::ParseUBXLine_NAVSVIN (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}

*/



// ------------------------------- UBX парсинг -----------------------------------
/*
bool TCANARDGPS::ParseUBX_OneLine (char *lpLine, unsigned long &d_size)
{
bool rv = false;
if (lpLine)
	{
	HDRUBX *lUbx = (HDRUBX*)lpLine;
	if (lUbx->Preamble_A == CUBXPREX_A && lUbx->Preamble_B == CUBXPREX_B)
		{
		// проверка контрольной суммы
		unsigned char *lSrc = &lUbx->Class;
		unsigned short sz = sizeof(HDRUBX) - 2 + lUbx->Len;
		unsigned long cur_sz = 0;
		TUBXCRC ubx_Crc;
		CalculateUBX_CRC (lSrc, sz, &ubx_Crc);
		TUBXCRC *lfnd_Crc = (TUBXCRC*)(lpLine + sizeof(HDRUBX) + lUbx->Len);
		char *lpMessUBX = lpLine + sizeof (HDRUBX);
		if (lfnd_Crc->CK_A == ubx_Crc.CK_A && lfnd_Crc->CK_B == ubx_Crc.CK_B)
			{
			// контрольнная сумма совпала
			// проверяем типовые сообщения
			cur_sz = lUbx->Len + sizeof(HDRUBX) + sizeof(TUBXCRC);
				
			switch (lUbx->Class)
				{
				case EUBXCLASS_NAV:
					{
					switch (lUbx->Id)
						{
						// плюшка для qground
						case EUBXNAV_VELNED:
						case EUBXNAV_SAT:
							{
							rv = true;
							break;
							}
						case EUBXNAV_DOP:					// для ровера и станции
							{
							rv = ParseUBXLine_DOP (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_PVT:					// для ровера и станции
							{
							rv = ParseUBXLine_PVT (lpMessUBX, lUbx->Len);
							break;
							}
						default:
							{
							rv = false;
							break;
							}
						}
					break;
					}
				case EUBXCLASS_MON:
					{
					// плюшка для qground
					if (lUbx->Id == EUBXMON_RF) rv = true;
					break;
					}		
				case EUBXCLASS_ACK:
					{
					switch (lUbx->Id)
						{
						case EUBXACK_NACK:
							{
							rv = ParseUBXLine_NACK (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXACK_ACK:
							{
							rv = ParseUBXLine_ACK (lpMessUBX, lUbx->Len);
							break;
							}
						}
					break;
					}
				default:
					{
					rv = rv;
					break;
					}
				}
				
			}
		if (rv) d_size = cur_sz;
		}
	}
return rv;
}
*/



void TCANARDGPS::CheckUpdateDOP (unsigned short inp_val, TTDOPTAG &dopval)
{
	if (inp_val && inp_val < 10000)
		{
		float dat = (float)inp_val;
		dopval.value = dat * (float)0.01;
		dopval.f_valid = true;
		}
	else
		{
		dopval.f_valid = false;
		dopval.value = C_DOP_NANVALUE;
		}
}



bool TCANARDGPS::ParseUBXLine_DOP (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXDOP))
		{
		FUBXDOP *ldop = (FUBXDOP*)lTxt;
		// PDOP parse
		CheckUpdateDOP (ldop->pDOP, GPSAccData.PDOP);
		// VDOP parse
		CheckUpdateDOP (ldop->vDOP, GPSAccData.VDOP);
		// HDOP parse
		CheckUpdateDOP (ldop->hDOP, GPSAccData.HDOP);	
		// EDOP parse
		CheckUpdateDOP (ldop->eDOP, GPSAccData.EDOP);
		// TDOP parse
		CheckUpdateDOP (ldop->tDOP, GPSAccData.TDOP);
		// GDOP parse
		CheckUpdateDOP (ldop->gDOP, GPSAccData.GDOP);	
		// NDOP parse
		CheckUpdateDOP (ldop->nDOP, GPSAccData.NDOP);
		rv = true;
		f_new_aux_data = rv;
		}
return rv;	
}



void TCANARDGPS::CheckUpdateVelocity (long inp_val, TTVELOCITYTAG &dopval)
{
	if (inp_val)
		{
		float dat = (float)inp_val;
		dopval.value_m_s = dat * (float)0.001;
		dopval.f_valid = true;
		}
	else
		{
		dopval.f_valid = false;
		dopval.value_m_s = 0;
		}
}




bool TCANARDGPS::ParseUBXLine_NACK (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes == sizeof(FUBXNACK))
	{
	FUBXNACK *lnack = (FUBXNACK*)lTxt;
	switch (lnack->clsID)
		{
		default:
			{
			rv = true;
			break;
			}
		}
	rv = true;
	}
return rv;
}



bool TCANARDGPS::ParseUBXLine_ACK (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes == sizeof(FUBXNACK))
	{
	FUBXNACK *lnack = (FUBXNACK*)lTxt;
	switch (lnack->clsID)
		{
		default:
			{
			rv = true;
			break;
			}
		}
	rv = true;
	}
return rv;
}



//volatile static bool sst_rtk = false;


bool TCANARDGPS::ParseUBXLine_PVT (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXPVT))
		{
		FUBXPVT *lpvt = (FUBXPVT*)lTxt;
		float dat;
		CheckUpdateVelocity (lpvt->velN, GPSAccData.VELN);
		CheckUpdateVelocity (lpvt->velE, GPSAccData.VELE);
		CheckUpdateVelocity (lpvt->velD, GPSAccData.VELD);
			
		if (lpvt->flags & 2)
			{
				Timer_RTK_implement = C_TIME_LED_RTK_STATUS;
				//sst_rtk = true;
			}
			
		if (GPSAccData.VELN.f_valid)
			{
			dat = lpvt->hAcc;
			dat *= (float)0.001;
			GPSAccData.COVARIANCES.PosCovr[0].f_valid = true;
			GPSAccData.COVARIANCES.PosCovr[0].value = dat;
			}
		else
			{
			GPSAccData.COVARIANCES.PosCovr[0].f_valid = false;
			GPSAccData.COVARIANCES.PosCovr[0].value = 0;
			}
			
		if (GPSAccData.VELE.f_valid)
			{
			dat = lpvt->hAcc;
			dat *= (float)0.001;
			GPSAccData.COVARIANCES.PosCovr[1].f_valid = true;
			GPSAccData.COVARIANCES.PosCovr[1].value = dat;
			}
		else
			{
			GPSAccData.COVARIANCES.PosCovr[1].f_valid = false;
			GPSAccData.COVARIANCES.PosCovr[1].value = 0;
			}
				
		if (GPSAccData.VELD.f_valid)
			{
			dat = lpvt->vAcc;
			dat *= (float)0.001;
			GPSAccData.COVARIANCES.PosCovr[2].f_valid = true;
			GPSAccData.COVARIANCES.PosCovr[2].value = dat;
			}
		else
			{
			GPSAccData.COVARIANCES.PosCovr[2].f_valid = false;
			GPSAccData.COVARIANCES.PosCovr[2].value = 0;
			}
			
			
		bool f_fix_novalid = false;
		
		if (lpvt->flags & 1)
			{
			//GPSAccData.height_ellipsoid_mm = lpvt->height;
			//GPSAccData.height_msl_mm = lpvt->hMSL;
			GPSAccData.SatelitAmount = lpvt->numSV;
			GPSAccData.SatelitViewN = lpvt->numSV;
				
			//GPSAccData.f_height_elips_valid = true;
			//GPSAccData.f_height_msl_valid = true;
			GPSAccData.f_FixMode_valid = true;
			GPSAccData.f_SatelitNumb_valid = true;
			GPSAccData.f_SatViewN_valid = true;
			}
		else
			{
			f_fix_novalid = true;
			}
	
		//if (lpvt->flags3 & 1) f_fix_novalid = true;		//проверка флаг невалидности
		if (f_fix_novalid)
			{
			//GPSAccData.f_height_elips_valid = false;
			//GPSAccData.f_height_msl_valid = false;
			GPSAccData.f_FixMode_valid = false;
			//GPSAccData.f_height_elips_valid = false;
			//GPSAccData.f_height_msl_valid = false;
			GPSAccData.f_SatelitNumb_valid = false;
			GPSAccData.f_SatViewN_valid = false;
			GPSAccData.Mode2D3D = 0;
			}
		else
			{
			unsigned char c_fix_mode = lpvt->fixType;
			if (c_fix_mode > 3) c_fix_mode = 1;
			GPSAccData.Mode2D3D = c_fix_mode;
			}
			
		dat = lpvt->sAcc; 
		dat *= (float)0.001;
		GPSAccData.COVARIANCES.VelCovr.value = dat;
		GPSAccData.COVARIANCES.VelCovr.f_valid = true;
				
		rv = true;
		}
return rv;	
}





unsigned long TCANARDGPS::crc24q_hash(unsigned char *data, unsigned short len)
{
    short i;
    unsigned long crc = 0;
    for (i = 0; i < len; i++) {
			crc = (crc << 8) ^ crc24q[data[i] ^ (unsigned char)(crc >> 16)];
			}
    crc = (crc & 0x00ffffff);
    return crc;
}



void TCANARDGPS::crc24q_sign (unsigned char *data, short len)
{
    unsigned long crc = crc24q_hash(data, len);
    data[len] = HI(crc);
    data[len + 1] = MID(crc);
    data[len + 2] = LO(crc);
}



bool TCANARDGPS::crc24q_check(unsigned char *data, short len)
{
    unsigned long crc = crc24q_hash(data, len - 3);
    return (((data[len - 3] == HI(crc)) && (data[len - 2] == MID(crc)) && (data[len - 1] == LO(crc))));
}





bool TCANARDGPS::GetInjectRTCMBufer (TBUFPARAM *lprm)
{
bool rv = false;
if (lprm && !GetTxStatus()) {
	lprm->lRam = buf_tx;
	lprm->sizes = sizeof(buf_tx);
	rv = true;
	}
return rv;	
}



bool TCANARDGPS::TransmitRTCM (TBUFPARAM *ltxprm)
{
bool rv = false;
if (ltxprm) {
	if (ltxprm->lRam && ltxprm->sizes) {
		RawTransmit (ltxprm->lRam, ltxprm->sizes);
		rv = true;
		}
	}
return rv;	
}



uint8_t TCANARDGPS::GetCurentSat_count ()
{
	return cur_sat_amount;
}



uint8_t TCANARDGPS::GetDimensModeGPS ()
{
	return gpsDmode;
}



bool TCANARDGPS::GetRTK_implement_status ()
{
	bool rv = false;
	if (Timer_RTK_implement) rv = true;
	return rv;
}

