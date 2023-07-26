#include "TFMINILIDAR.h"


bool TUSARTLIDAR::f_new_lidar_distanse = false;
float TUSARTLIDAR::curent_metter;


TUSARTLIDAR::TUSARTLIDAR ()
{
sw = 0;
f_sync = false;

clear_recv_buf ();
f_new_lidar_data_uav = false;
f_new_lidar_data_feed = false;
ClearRxBufers ();
#ifdef DEBUG_LIDAR
	lidar_good_frame_cnt = 0;
	lidar_bad_crc_frame_cnt = 0;
	lidar_bad_frame_cnt = 0;
	lidar_bad_many_sync_frame_cnt = 0;
	lidar_bad_over_frame_cnt = 0;
	lidar_alignment_frame_cnt = 0;
#endif
	c_rate_period = C_DEF_LIDAR_RATE;
	timer_rate = 0;
	SYSBIOS::ADD_TIMER_SYS (&timer_rate);
}



bool TUSARTLIDAR::CheckFrame ()
{
	bool rv = false;
	
	return rv;
}



void TUSARTLIDAR::SetRate_uav (float hz_val)
{
	if (hz_val > C_MAXPERIODHZ_LIDAR) hz_val = C_MAXPERIODHZ_LIDAR;
	if (hz_val < C_MINPERIODHZ_LIDAR) hz_val = C_MINPERIODHZ_LIDAR;
	float period = 1.0 / hz_val / 0.001;
	c_rate_period = period;
}



void TUSARTLIDAR::Task ()
{
	SLIDATFRAME data;
	if (f_enable)
		{
		if (PopFrame (&data))
			{
			unsigned short cm_range = data.Dist_H; cm_range <<= 8;
			cm_range |= data.Dist_L;
			//cur_lidar_data.max_dist = 12.0;
			//cur_lidar_data.min_dist = 0.1;
				
			cur_lidar_data.range = cm_range;
			cur_lidar_data.range /= 100;		// переводим в метры
			curent_metter = cur_lidar_data.range;
			f_new_lidar_data_uav = true;
			f_new_lidar_data_feed = true;
			f_new_lidar_distanse = true;
			}
		}
	
}



void TUSARTLIDAR::Init (BUFPAR *rx_b, BUFPAR *tx_b)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//USART_InitTypeDef USART_InitStructure;
	USARTPort = C_USART_LIDAR_PORT;
	rx_buf = *rx_b; tx_buf = *tx_b;
	
	
	USART_DeInit(USARTPort);
	
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);

	f_tx_status = false;

	// RX pin
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  // TX pin
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	SetSpeed (115200);
	
  USART_ITConfig (USARTPort, USART_IT_RXNE, ENABLE);
  USART_Cmd (USARTPort, ENABLE);
	
	lFS_PUSART[C_LIDAR_INDX_PORT] = (TUSARTIFB*)this;
	ClearRxBufers ();
	
	cur_lidar_data.max_dist = 12.0;
	cur_lidar_data.min_dist = 0.1;
}



void TUSARTLIDAR::ClearSlot (char ix)
{
if (ix >= 0 && ix < C_LIDARBUF_AMOUNT)
	{
	frame[ix].full = false;
	}
}



char TUSARTLIDAR::LidarCRC (void *lp, unsigned char size)
{
	char crc = 0;
	char *lData = (char *)lp;
	while (size)
		{
		crc += *lData;
		lData++;
		size--;
		}
	return crc;
}



// ищем свободный буфер впереди и заполняем данными
bool TUSARTLIDAR::PushFrame (SLIDATFRAME *ldat)
{
bool rv = false;
	if (ldat)
		{
		if (frame_tx_indx >= C_LIDARBUF_AMOUNT) frame_tx_indx = 0;
		char fnd_ix = -1;
		char cnt = 0;
		char start_ix = frame_tx_indx;
		while (cnt < C_LIDARRECVBUF_SIZE)	
			{
			if (!frame[start_ix].full)
				{
				fnd_ix = cnt;
				break;
				}
			start_ix++;
			if (start_ix >= C_LIDARBUF_AMOUNT) start_ix = 0;
			cnt++;
			}
		if (fnd_ix >=0)
			{
			frame[fnd_ix].slot = *ldat;
			frame[fnd_ix].full = true;
			rv = true;
			frame_tx_indx = fnd_ix;
			}
		}
return rv;
}





SLIDATFRAME *TUSARTLIDAR::CheckLidarData ()
{
SLIDATFRAME *rv = 0;
char ix_inc = 0;
SLIDATFRAME *fnd_frame;
if (f_enable)
	{
	while (ix_inc <= C_RECVBUF_TAILSIZE)
		{
		fnd_frame = (SLIDATFRAME*)(&recv_buf[ix_inc]);


		// подсчитываем контрольную сумму
		if (LidarCRC (fnd_frame, sizeof(SLIDATFRAME)-1) == fnd_frame->Crc8)
			{
			rv = fnd_frame;
			#ifdef DEBUG_LIDAR
				if (ix_inc) lidar_alignment_frame_cnt++;
			#endif
			break;
			}
		else
			{
			#ifdef DEBUG_LIDAR
				lidar_bad_crc_frame_cnt++;
			#endif
			}



		ix_inc++;
		}
	}
	
return rv;
}




bool TUSARTLIDAR::GetLidarData_feed (LIDOUTDATA *lout)
{
bool rv = false;
	if (lout && f_enable && f_new_lidar_data_feed)
		{
		*lout = cur_lidar_data;
		f_new_lidar_data_feed = false;
		rv = true;
		}

return rv;
}


bool TUSARTLIDAR::GetLidarData_uav (LIDOUTDATA *lout)
{
bool rv = false;
if (!timer_rate)
	{
	if (lout && f_enable && f_new_lidar_data_uav)
		{
		*lout = cur_lidar_data;
		rv = true;
		f_new_lidar_data_uav = false;
		}
	timer_rate = c_rate_period;
	}
return rv;
}


bool TUSARTLIDAR::GetDistance (float &value)
{
bool rv = false;
if (f_new_lidar_distanse)
	{
	value = curent_metter;
	f_new_lidar_distanse = false;
	rv = true;
	}
return rv;	
}


/*
В описании протокола выдачи данных, нет временных характеристик описывающих межпакетные интервалы для синхронизации кадров. 
Поэтому данные с лидара синхронизируем методом анализа байтового потока.
В методе байтовой синхронизации есть четыре варианта примема для протокола лидара:

1. Вариант... Когда принято 2 байта 0x59 - самый простой.
2. Вариант... принято 3 байта 0x59  (два из которых atr, а один может crc из прошлого пакета).
3. Вариант... принято 3 байта 0x59  (два из которых atr, а один Dist_L).
4. Вариант... принято 4 байта 0x59 - тут один вариант - ( это прошлый crc = 0x59, два байта atr=0x59 и байт Dist_L = 0x59).
----------------------
Итог:
 По неправильной синхронизации может быть принят один лишний байт прошлого кадра - crc, в том случае если он был равен 0x59

*/
void TUSARTLIDAR::ISR_DataRx (unsigned char dat)
{
if (f_enable)		// frame_tx_indx >= 0 && frame_tx_indx < C_LIDARBUF_AMOUNT
	{
	if (recv_b_ix < C_LIDARRECVBUF_SIZE)
		{
		if (recv_sync)
			{
			recv_buf[recv_b_ix] = dat;
			recv_b_ix++;
			if (recv_b_ix >= sizeof(SLIDATFRAME))
				{
				// считаем контрольную сумму и заполняем буфера
				SLIDATFRAME *lFr = CheckLidarData ();			// проверяем контрольную сумму
				if (lFr) 
					{
					PushFrame (lFr);									// заполняем свободный приемный буфер
					#ifdef DEBUG_LIDAR
						lidar_good_frame_cnt++;
					#endif
					}

				// идем на синхронизацию
				clear_recv_buf ();
				}
			}
		else
			{
			if (dat == C_LIDAR_FRAMESYNCATTR_BYTE)	// процесс синхронизации: записываем и подсчитываем 0x59
				{
				if (recv_atr_ix < sizeof(SLIDATFRAME))
					{
					recv_buf[recv_atr_ix++] = dat;
					}
				else
					{
					// ошибка - уходит на синхронизацию
					#ifdef DEBUG_LIDAR
						lidar_bad_many_sync_frame_cnt++;
					#endif
					clear_recv_buf ();
					}
				}
			else
				{
				if (recv_atr_ix >= 2 && (recv_atr_ix <= (sizeof(SLIDATFRAME) - 2)))		// может быть что подряд будет идти 7 байт со значением 0x59
					{
					recv_buf[recv_atr_ix++] = dat;
					recv_sync = true;
					recv_b_ix = recv_atr_ix;
					}
				else
					{
					recv_atr_ix = 0;
					}
				}
			}
		}
	else
		{
		// ошибка - уходит на синхронизацию
		#ifdef DEBUG_LIDAR
			lidar_bad_over_frame_cnt++;
		#endif
		clear_recv_buf ();
		}
	}
}



void TUSARTLIDAR::clear_recv_buf ()
{
		recv_b_ix = 0;
		recv_atr_ix = 0;
		recv_sync = false;
}



void TUSARTLIDAR::ClearRxBufers ()
{
char ix = 0;
	while (ix < C_LIDARBUF_AMOUNT)
		{
		frame[ix].full = false;
		ix++;
		}
	frame_tx_indx = 0;
	frame_rx_indx = 0;
}



// просматривает приемные буфера и извлекает из первого найденного данные
bool TUSARTLIDAR::PopFrame (SLIDATFRAME *ldat)
{
bool rv = false;
	if (ldat)
		{
		char ix = 0;
		if (frame_rx_indx >= C_LIDARBUF_AMOUNT) frame_rx_indx = 0;		// стартовый указатель просмотра на чтение буферов
		char search_ix = frame_rx_indx;
		while (ix < C_LIDARBUF_AMOUNT)
			{
			if (frame[search_ix].full)
				{
				*ldat = frame[search_ix].slot;
				frame[search_ix].full = false;
				search_ix++;
				frame_rx_indx = search_ix;
				if (frame_rx_indx >= C_LIDARBUF_AMOUNT) frame_rx_indx = 0;
				rv = true;
				break;
				}
			ix++;
			}
		}
return rv;
}



void TUSARTLIDAR::SetEnabled (bool val)
{
f_enable = val;
}



