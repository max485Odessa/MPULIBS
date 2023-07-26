#include "gps_class.h"
#include <string.h>
#include <math.h>


const char *ltxt_gngga = "$GNGGA";
const char *ltxt_gnss = "$GNGNS";
const char *ltxt_gpgns = "$GPGNS";
const char *ltxt_gpgga = "$GPGGA";
const char *ltxt_gnrmc = "$GNRMC";
const char *ltxt_gprmc = "$GPRMC";
const char *ltxt_gngsa = "$GNGSA";
const char *ltxt_gpgsa = "$GPGSA";
const char *ltxt_gngll = "$GNGLL";
const char *ltxt_gpgll = "$GPGLL";
const char *ltxt_gpgsv = "$GPGSV";
//const char *ltxt_gpgsv = "$GNGSV";


char TCANARDGPS::bufer_p[C_GPSRECVBUF_SIZE];
//utimer_t TCANARDGPS::Timer_RTK_implement = 0;


void wgsllh2ecef (double lat, double lon, double alt , ECEF_POINT &point)
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



void wgsecef2llh(ECEF_POINT &ecef, WGS84POINT &llh)
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

    /* Update step. */
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





uint8_t TCANARDGPS::cur_sat_amount = 0;
uint8_t TCANARDGPS::gpsDmode = 0;


TCANARDGPS::TCANARDGPS ()
{
	nmea_size = 0;
	f_new_gps_data = false;
	NeedSyncFrame ();
	C_PERIOD_PVT = 2000;
	
	SYSBIOS::AddPeriodicThread_A (this, (void*)LowHard_CB, 1);		// 1ms
	
	DopsTimer = 0;
	SYSBIOS::ADD_TIMER_SYS (&DopsTimer);
	ErrorTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&ErrorTimeout);
	
	UBXReqTimet_DOP = 0;
	SYSBIOS::ADD_TIMER_SYS (&UBXReqTimet_DOP);
	UBXReqTimet_PVT = 0;
	SYSBIOS::ADD_TIMER_SYS (&UBXReqTimet_PVT);
	UBXReqTimet_RATE = 0;
	SYSBIOS::ADD_TIMER_SYS (&UBXReqTimet_RATE);
	//SYSBIOS::ADD_TIMER_SYS (&Timer_RTK_implement);
	
	
	memset (&GPSAccData, 0, sizeof(GPSAccData));
	speed_search = EUSARTSPD_460800;
	DopsTimer = C_SPEEDSET_TIMEOUT;
	
	cur_sat_amount = 0;
	gpsDmode = 0;
	SWVal = EGPSSW_SPEED_DETECT_A;
	AddObjectToExecuteManager ();
}



void TCANARDGPS::SetRate_PVT (unsigned long v_pvt)
{
	C_PERIOD_PVT = v_pvt;
	UBXReqTimet_RATE = 0;
}



bool TCANARDGPS::CalculateUBX_CRC (unsigned char *lSrc, unsigned short sz, TUBXCRC *lDstCrc)
{
	bool rv = false;
	unsigned char CK_A = 0;
	unsigned char CK_B = 0;
	unsigned char *lEnd = (unsigned char*)bufer_p + C_GPSRECVBUF_SIZE - 1;
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
lFrame->portID = 1;					
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



void TCANARDGPS::ClearBufer ()
{
memset (bufer_p, 0, sizeof(bufer_p));
b_indx = 0;
f_is_recived_data = false;
}



void TCANARDGPS::NeedSyncFrame ()
{
f_sync_success = false;
LastdataTimeout = 0;
ClearBufer ();
}



bool TCANARDGPS::check_recive_data ()
{
return f_is_recived_data;	
}





void TCANARDGPS::Init (unsigned long speed_set)	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	huart.Instance = C_GPS_USART;
	nvic_irq = C_GPS_NVIC_IRQ;
	lFS_PUSART[EUSARTIX_GPS] = (TUSART_IFACE*)this;
	
	if (!speed_set) 
		{
		if (speed_search >= EUSARTSPD_MAX) speed_search = EUSARTSPD_9600;
		speed_set = C_USARTSPEED_INDX[speed_search];
		}
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE ();
	
	tx_size = 0;
	f_tx_status = false;
	
	GPIO_InitStructure.Pin = C_GPS_PIN_TX | C_GPS_PIN_RX;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init (C_GPS_GPIO_PORT, &GPIO_InitStructure);
	
	Set_usart_speed (speed_set);
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits = UART_STOPBITS_1;
  huart.Init.Parity = UART_PARITY_NONE;
  huart.Init.Mode = UART_MODE_TX_RX;
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_8;//UART_OVERSAMPLING_16;
  huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_MSBFIRST_INIT;//UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart);
	
	
	NVIC_USART_ENABLE (true);				// разрешить прерывания
	USART_ITConfig_RX (true);	
  USART_Cmd (ENABLE);

}



void TCANARDGPS::UBX_SpeedSet_460800 ()
{
SendUBX_SetSpeed (C_NEED_WORK_SPEED);
}



void TCANARDGPS::ISR_DataRx (unsigned char dat)
{
	LastdataTimeout = 0;			// сброс таймаута покоя линии приема (появилась активность)
	if (f_sync_success)
		{
		if (!f_is_recived_data)
			{
			if (b_indx < sizeof(bufer_p)) bufer_p[b_indx++] = dat;
			}
		}
}


/*
bool TUSART_IFACE::ISR_DataTx (unsigned char *lDat)
{
bool rv = false;
if (tx_size)
	{
	rv = true;
	lDat[0] = lTxAdr[0];
	lTxAdr++;
	tx_size--;
	}
else
	{
	f_tx_status = false;
	}
return rv;
}



bool TUSART_IFACE::GetTxStatus () 
{
return !f_tx_status;
}



void TUSART_IFACE::RawTransmit (unsigned char *lTx, unsigned short sizes)
{
	lTxAdr = lTx;
	tx_size = sizes;
	if (sizes) 
		{
		f_tx_status = true;
		USART_ITConfig(USARTPort, USART_IT_TXE, ENABLE);
		}
}
*/



// static
void TCANARDGPS::LowHard_CB (TCANARDGPS *lThis)
{
	lThis->PeriodicCall_1Ms ();
}



void TCANARDGPS::DataInBuferStatus ()
{
	nmea_size = b_indx;	// запомнить размер данных в буфере
	f_is_recived_data = true;		// заблокировать прием - установив флаг наличия данных для парсинга
}



void TCANARDGPS::PeriodicCall_1Ms ()
{
	LastdataTimeout++;
	if (LastdataTimeout >= C_GPS_TIMEOUT_FRAME)
		{
		f_sync_success = true;
		if (b_indx) 
			{
			DataInBuferStatus ();
			}
		else
			{
			ClearBufer ();
			}
		LastdataTimeout = 0;
		}
	//TimerCBControl = C_PERIODIC_CBTIME;	// следующий вызов через 1 ms интервал
}



void TCANARDGPS::NextSpeedSearch ()
{
char cspd = speed_search; cspd++; 
if (cspd >= EUSARTSPD_MAX) cspd = EUSARTSPD_9600; 
speed_search = (EUSARTSPD)cspd;
Init (C_USARTSPEED_INDX[speed_search]);		
UBX_SpeedSet_460800 ();
}



// 
void TCANARDGPS::ToDetectUsartSpeed ()
{
	SWVal = EGPSSW_SPEED_DETECT_A;
	NextSpeedSearch ();
	DopsTimer = C_SPEEDSET_TIMEOUT;
}


void TCANARDGPS::Task ()
{
	//rtcm3_task ();							// проверка доступности rtcm3 пакетов
	if (f_tx_status) return;
	//if (TUAVCAN::GetNodeMode() == E_NDMODE_SOFTWARE_UPDATE) return;
	// машина состояния
	// EGPSSW_SPEED_DETECT_A - детектирование пакетов на выбраной скорости USART и переключение на следующую (установка приоритетной скорости)
	// EGPSSW_WORK - установлена приоритетная скорость - работаем нормально
	switch (SWVal)
		{
		case EGPSSW_SPEED_DETECT_A:		// ждем по таймауту данные
			{
			if (DopsTimer)
				{
				if (check_recive_data ())
					{
					unsigned short pars_cnt = Parse ();
					if (pars_cnt >= 3)	// доп проверка по наличию хотя-бы 3-х сообщений в принятом кадре
						{
						if (speed_search == EUSARTSPD_460800)		// EUSARTSPD_460800
							{
							// если пакеты обнаружены на нужной скорости - перейти на обычную работу
							SWVal = EGPSSW_WORK;
							ErrorTimeout = C_SPEEDSET_TIMEOUT;
							DopsTimer = 100;
							// после установки скорости запрограмировать gps на нужные параметры
							UBXReqTimet_DOP = 0;
							UBXReqTimet_PVT = 0;
							UBXReqTimet_RATE = 0;
							swUBXreq = ENEXTREQ_DOP;
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
					ClearBufer ();
					}
				}
			else
				{
				// время поиска пакетов на текущей скорости истекло, переходим на следующюю скорость поиска
				NextSpeedSearch ();
				ClearBufer ();
				DopsTimer = C_SPEEDSET_TIMEOUT;							// таймаут ожидания приема пакетов на установленной скорости
				}
			break;
			}
		case EGPSSW_WORK:
			{
			if (check_recive_data ())
				{
				unsigned short pars_cnt = Parse ();
				if (pars_cnt) 
					{
					ErrorTimeout = C_GPS_NODATA_TIMEOUT;	// данные есть, сбрасываем таймаут таймер ошибки
					f_new_gps_data = true;
					}

				if (ErrorTimeout)
					{
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
									unsigned long cur_period = TEASYMEMSTORAGE::GetRawParam (ESAVEPARIX_FIX_RATE);
									if (cur_period < C_FIX_RATE_MIN) cur_period = C_FIX_RATE_MIN;
									SendUBX_CFG_RATE_req (cur_period);
									DopsTimer = C_MATTX_TIMEOUT;
									UBXReqTimet_RATE = C_PERIOD_RATE * 5;
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
					// данные не поступали в течении отведенного таймаута, переходим к поиску модуля на других скоростях
					ToDetectUsartSpeed ();
					}
				ClearBufer ();
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



char *TCANARDGPS::FindFistDelimitter (char *lInpInBuf, char delimm)
{
char *lpRv = 0;
if (nmea_size)
	{
	char *lEnd = bufer_p + nmea_size;
	char dat;
	while (lEnd > lInpInBuf)
		{
		dat = lInpInBuf[0];
		if (!dat) break;
		if (dat == delimm)
			{
			lpRv = lInpInBuf;
			break;
			}
		lInpInBuf++;
		}
	}
return lpRv;
}





char *TCANARDGPS::GetFistStringTag (unsigned long *cut_sz, EGPSMESSTYPE &typetag)
{
	char *lpRv = 0;
	if (nmea_size)
		{
		lCurFindAdr = bufer_p;
		CurFindTagSize = 0;
		lpRv = GetNextStringTag (cut_sz, typetag);
		}
	return lpRv;
}









bool TCANARDGPS::CheckUBX_Format (HDRUBX *lframe, unsigned long *cut_sz)
{
	bool rv = false;
	if (lframe)
		{
		char *lEnd = bufer_p + C_GPSRECVBUF_SIZE - 1;
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



char *TCANARDGPS::GetNextStringTag (unsigned long *cut_sz, EGPSMESSTYPE &typetag)
{
	char *lpRv = 0;
	typetag = EGPSMESSTYPE_NONE;
	if (nmea_size)
		{
		lCurFindAdr += CurFindTagSize;
		char *lEnd = bufer_p + nmea_size - 6;	
		
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
					lCurFindAdr++;
					}
				}
			}
		if (cut_sz && lpRv) *cut_sz = CurFindTagSize;
		}
	return lpRv;
}



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
				if (ParseNMEA_OneLine (lTagLine, size_line))
					{
						
					}
				else
					{
					f_error = true;	
					}
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










bool TCANARDGPS::ParseNMEA_OneLine (char *lTagLine, unsigned long &d_size)
{
	bool rv = false;
	unsigned long FieldLen;			// размер поля в строке
	unsigned long TagMaxCount;	
	unsigned long size_line = 0;
	char *lFieldStr;
	if (lTagLine)
		{
		if (CheckNMEA_Format (lTagLine, &size_line))
			{
			GetTagStringDelimIndx (lTagLine, 0xFF, ',', &lFieldStr, &TagMaxCount);		// определяет количество полей в строке отчета
			FieldLen = GetTagStringDelimIndx (lTagLine, 0, ',', &lFieldStr, 0);				// получение вхождения в название поля и подсчет размера текстового поля
			if (FieldLen >= 4)	// размерность поля типа отчета (по крайней мере больше или равно 4)
				{
				do 	{	// из необходимого нужны только GGA, RMC, GSA, GSV
						if (str_compare((char*)ltxt_gnss, lFieldStr, FieldLen))
							{
							ParseGNSSLine (lTagLine, size_line, TagMaxCount);		// парсинг GNSS сообщения
							break;
							}
							
						if (str_compare((char*)ltxt_gpgns, lFieldStr, FieldLen))
							{
							ParseGPGNSLine (lTagLine, size_line, TagMaxCount);		// парсинг GNSS сообщения
							break;
							}

							
						/*
						if (str_compare((char*)ltxt_gngga, lFieldStr, FieldLen))
							{
							ParseGGALine (lTagLine, size_line, TagMaxCount);		// запустить парсинг GGA
							break;
							}
						*/
						
						if (str_compare((char*)ltxt_gnrmc, lFieldStr, FieldLen))
							{
							ParseRMCLine (lTagLine, size_line, TagMaxCount); 	// запустить парсинг RMC
							break;
							}		
						if (str_compare((char*)ltxt_gngsa, lFieldStr, FieldLen))
							{
							ParseGSALine (lTagLine, size_line, TagMaxCount); 	// запустить парсинг GSA
							break;
							}
						/*
						if (str_compare((char*)ltxt_gpgsv, lFieldStr, FieldLen))
							{
							ParseGSVLine (lTagLine, size_line, TagMaxCount); 	// запустить парсинг GSV
							break;
							}
						*/
						} while (false);
				rv = true;
				}
			}
		}
	return rv;
}


/*
unsigned char TCANARDGPS::ParseNMEA_Frames ()
{
	unsigned char rv = 0;
	unsigned long size_line = 0;
	unsigned short FrameLineAmount = 0;
	char *lTagLine = GetFistStringTag (&size_line);
	char *lFieldStr;
	unsigned long FieldLen;			// размер поля в строке
	unsigned long TagMaxCount;	
	while (lTagLine)
		{
		FrameLineAmount++;
		GetTagStringDelimIndx (lTagLine, 0xFF, ',', &lFieldStr, &TagMaxCount);	// определяет количество полей в строке отчета
		if (TagMaxCount >= 2)		// проверка на количество полей в строке (ну по крайней мере больше 2)
			{
			FieldLen = GetTagStringDelimIndx (lTagLine, 0, ',', &lFieldStr, 0);			// получение вхождения в название поля и подсчет размера текстового поля
			if (FieldLen >= 4)	// размерность поля типа отчета (по крайней мере больше или равно 4)
				{
				do 	{	// из необходимого нужны только GGA, RMC, GSA
						if (str_compare((char*)ltxt_gngga, lFieldStr, FieldLen))
							{
							if (ParseGGALine (lTagLine, size_line, TagMaxCount)) rv++;	// запустить парсинг GGA
							break;
							}
						if (str_compare((char*)ltxt_gnrmc, lFieldStr, FieldLen))
							{
							if (ParseRMCLine (lTagLine, size_line, TagMaxCount)) rv++; 	// запустить парсинг RMC
							break;
							}		
						if (str_compare((char*)ltxt_gngsa, lFieldStr, FieldLen))
							{
							if (ParseGSALine (lTagLine, size_line, TagMaxCount)) rv++; 	// запустить парсинг GSA
							break;
							}
						if (str_compare((char*)ltxt_gpgsv, lFieldStr, FieldLen))
							{
							if (ParseGSVLine (lTagLine, size_line, TagMaxCount)) rv++; 	// запустить парсинг GSV
							break;
							}
						} while (false);
				}
			}
		lTagLine = GetNextStringTag (&size_line);
		}
	return rv;
}
*/



bool TCANARDGPS::ParseGPGNSLine (char *lTxt, unsigned long size, unsigned long amount_filed)
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
		
		// quality parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_QUAL, ',', &lFieldStr, 0);
		ParseFieldPosMode (lFieldStr, LenField);		// ParseFieldQuality
		
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
		
		// quality parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_QUAL, ',', &lFieldStr, 0);
		ParseFieldPosMode (lFieldStr, LenField);		// ParseFieldQuality
		
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
			
		// vdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_VDOP, ',', &lFieldStr, 0);
		rv |= ParseFieldVDOP (lFieldStr, LenField);
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
			//if (dat == 'F' || dat == 'R') Timer_RTK_implement = C_TIME_LED_RTK_STATUS;
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
		GPSAccData.NmeaAltitude = outd;
		rv = true;
		}
	}
GPSAccData.f_Nmea_Altitude_valid = rv;
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
		GPSAccData.NmeaDifElipsHeight = outd;
		rv = true;
		}
	}
GPSAccData.f_Nmea_DifElipsHeight_valid = rv;
return rv;
}



bool TCANARDGPS::GetData (uavcan_equipment_gnss_Fix &datt)
{
	bool rv = CheckNewData ();
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
				// если спутников меньше либо равно 3 или нет значений высоты, то mode никак не может быть 3D
				if (GPSAccData.SatelitAmount <= 3 || (!GPSAccData.f_Nmea_Altitude_valid && !GPSAccData.f_height_msl_valid)) datt.status = 2;
				}
			}
		else
			{
			datt.status = 0;
			}
		gpsDmode = datt.status;
			
		if (GPSAccData.f_Nmea_DifElipsHeight_valid && GPSAccData.f_Nmea_Altitude_valid)	// елипсоидная высота высчитывается из высоты над морем
			{
			float elips_h = (GPSAccData.NmeaAltitude + GPSAccData.NmeaDifElipsHeight) * 1000;		// метры в милиметры
			datt.height_ellipsoid_mm = elips_h;
			}
		else
			{
			if (GPSAccData.f_height_elips_valid)
				{
				datt.height_ellipsoid_mm = GPSAccData.height_ellipsoid_mm;
				}
			else
				{
				datt.height_ellipsoid_mm  = 0;
				}
			}

		if (GPSAccData.f_Nmea_Altitude_valid)
			{
			float dat = GPSAccData.NmeaAltitude * 1000;		// метры в милиметры
			datt.height_msl_mm = dat;
			}
		else
			{
			if (GPSAccData.f_height_msl_valid)
				{
				datt.height_msl_mm = GPSAccData.height_msl_mm;
				}
			else
				{
				datt.height_msl_mm = 0;
				}
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
			
		//rv = true;
		f_new_gps_data = false;
		}
	return rv;
}



TCOVRPARAMS *TCANARDGPS::GetCovariances ()
{
	TCOVRPARAMS *rv = 0;
	if (CheckNewData ()) rv = &GPSAccData.COVARIANCES;
	return rv;
}


// готово, все поля заполнены
bool TCANARDGPS::GetAuxData (uavcan_equipment_gnss_Auxiliary &datt)
{
	bool rv = CheckNewData ();
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



bool TCANARDGPS::CheckNewData ()
{
return f_new_gps_data;
}



void TCANARDGPS::ClearFlagNewData ()
{
 f_new_gps_data = false;
}




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





// ------------------------------- UBX парсинг -----------------------------------
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
						case EUBXNAV_VELNED:
						case EUBXNAV_SAT:
							{
							rv = true;
							break;
							}
						case EUBXNAV_POSLLH:		
							{
							rv = ParseUBXLine_NAVPOSLLH (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_RELPOSNED:		
							{
							rv = ParseUBXLine_NAVRELPOSNED (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_STATUS:			
							{
							rv = ParseUBXLine_NAVSTATUS (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_SVIN:			
							{ 
							rv = ParseUBXLine_NAVSVIN (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_DOP:					
							{
							rv = ParseUBXLine_DOP (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_PVT:				
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



void TCANARDGPS::CheckUpdateDOP (unsigned short inp_val, TTDOPTAG &dopval)
{
	if (inp_val && inp_val < 10000)
		{
		float dat = inp_val;
		dopval.value = dat * 0.01;
		dopval.f_valid = true;
		}
	else
		{
		dopval.f_valid = false;
		dopval.value = C_DOP_NANVALUE;
		}
}



bool TCANARDGPS::ParseUBXLine_DOP (char *lRam, unsigned long sizes)
{
bool rv = false;
	if (lRam && sizes == sizeof(FUBXDOP))
		{
		FUBXDOP *ldop = (FUBXDOP*)lRam;
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
		}
return rv;	
}



void TCANARDGPS::CheckUpdateVelocity (long inp_val, TTVELOCITYTAG &dopval)
{
	if (inp_val)
		{
		float dat = inp_val;
		dopval.value_m_s = dat * 0.001;
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
				//Timer_RTK_implement = C_TIME_LED_RTK_STATUS;
				//sst_rtk = true;
			}
			
		if (GPSAccData.VELN.f_valid)
			{
			dat = lpvt->hAcc;
			dat *= 0.001;
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
			dat *= 0.001;
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
			dat *= 0.001;
			GPSAccData.COVARIANCES.PosCovr[2].f_valid = true;
			GPSAccData.COVARIANCES.PosCovr[2].value = dat;
			}
		else
			{
			GPSAccData.COVARIANCES.PosCovr[2].f_valid = false;
			GPSAccData.COVARIANCES.PosCovr[2].value = 0;
			}
			

/*
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
*/

		if (!(lpvt->flags3 & 1))	//проверка флаг невалидности
			{
			GPSAccData.height_ellipsoid_mm = lpvt->height;
			GPSAccData.height_msl_mm = lpvt->hMSL;
			GPSAccData.f_height_elips_valid = true;
			GPSAccData.f_height_msl_valid = true;
			}
		else
			{
			GPSAccData.f_height_elips_valid = false;
			GPSAccData.f_height_msl_valid = false;
			}
			
		unsigned char c_fix_mode = lpvt->fixType;
		if (c_fix_mode > 3) c_fix_mode = 1;
		GPSAccData.f_FixMode_valid = true;
		GPSAccData.Mode2D3D = c_fix_mode;
		if (GPSAccData.f_height_elips_valid && GPSAccData.f_height_msl_valid)
			{
				GPSAccData.Mode2D3D = 3;
			}


		GPSAccData.SatelitAmount = lpvt->numSV;
		GPSAccData.SatelitViewN = lpvt->numSV;
		GPSAccData.f_SatelitNumb_valid = true;
		GPSAccData.f_SatViewN_valid = true;
			
		
			
		dat = lpvt->sAcc; 
		dat *= 0.001;
		GPSAccData.COVARIANCES.VelCovr.value = dat;
		GPSAccData.COVARIANCES.VelCovr.f_valid = true;
				
		rv = true;
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



