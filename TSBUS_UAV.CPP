#include "TSBUS_UAV.H"



TSBUSRECEIVER::TSBUSRECEIVER ()
{
	clear_rx ();
}



// у SBUS есть критерий синхронизации - это межпакетный интервал
// формирователь sbus фрейма синхронизируется по этому интервалу
void TSBUSRECEIVER::Isr_100mks ()
{
if (frame_marg_timeout)
	{
	frame_marg_timeout--;
	if (!frame_marg_timeout) rx_ix = 0; 	// синхронизируем новый sbus кадр
	}
}



void TSBUSRECEIVER::clear_rx ()
{
	rx_ix = 0;
}




void TSBUSRECEIVER::ISR_DataRx (unsigned char dat)
{
	if (rx_ix < sizeof(TSBUSPROTOCOL))
		{
		bufrx[rx_ix++] = dat;
		if (rx_ix == sizeof(TSBUSPROTOCOL))
			{
			if (!f_new_frame_isr) 
				{
				CopyMemorySDC ((char*)bufrx, (char*)&protocol_data_frame, sizeof(protocol_data_frame));
				f_new_frame_isr = true;			// кадр принят
				f_new_frame_check = false;	// но его нужно проверить
				}
			}
		}
	frame_marg_timeout = 10;
}



void TSBUSRECEIVER::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	huart.Instance = C_SBUS_USART;
	nvic_irq = C_SBUS_NVIC_IRQ;
	lFS_PUSART[EUSARTIX_SBUS] = (TUSART_IFACE*)this;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE ();
	
	GPIO_InitStructure.Pin = C_SBUS_PIN_RX;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF3_USART2;
	HAL_GPIO_Init (GPIOA, &GPIO_InitStructure);

	f_tx_status = false;
	Set_usart_speed (100000);
	
  huart.Init.WordLength = UART_WORDLENGTH_9B;
  huart.Init.StopBits = UART_STOPBITS_2;
  huart.Init.Parity = UART_PARITY_EVEN;
  huart.Init.Mode = UART_MODE_RX;	
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_8;//UART_OVERSAMPLING_16;
  huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_MSBFIRST_INIT;//UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart);
		
	NVIC_USART_ENABLE (true);				// разрешить прерывания
	USART_ITConfig_RX (true);	
  USART_Cmd (ENABLE);
	AddObjectToExecuteManager ();
}




unsigned short TSBUSRECEIVER::DigChannelValue (bool val)
{
	unsigned short rv = 2047;
	if (!val) rv = 0;
	return rv;
}




void TSBUSRECEIVER::Task ()
{
	if (f_new_frame_isr)
		{
		if (!f_new_frame_check)
			{
			if (protocol_data_frame.startbyte == C_SBUS_FIST_BYTE_VALUE && protocol_data_frame.endbyte == C_SBUS_LAST_BYTE_VALUE)
				{
				// переформатируем 11 бит в 16 битный массив
				SBUSDecodingArrays (protocol_data_frame.array, data_new.channels);
				data_new.flags = protocol_data_frame.flags;
				f_new_frame_check = true;		// кадр проверен
				}
			else
				{
				f_new_frame_isr = false;	// принимаем новый кадр - этот проверку не прошел	
				}
			}
		}
}



TSBUSDATA *TSBUSRECEIVER::GetData ()
{
	// при приеме данных заполнение data_new автоматически блокируется
	TSBUSDATA *rv = 0;
	if (f_new_frame_isr && f_new_frame_check) rv = &data_new;	
	return rv;
}



void TSBUSIFACE::UnBlockAfter_recv ()
{
if (f_new_frame_isr) 
	{
	f_new_frame_isr = false;	// это гарантирует что сброс произойдет тогда когда буфер уже заблокирован и не происходит формирования кадра
	}
}




void TSBUSRECEIVER::SBUSDecodingArrays (unsigned char *lBase, unsigned short Dst[16])
{
unsigned char mask_inp = 1, cnt_loc;
unsigned char out_rc_ix = 0;
unsigned short dat_out, mask_out;
unsigned char dat_in = *lBase;
while (out_rc_ix < 16)
    {
    cnt_loc = C_SBUS_RCBITS_SIZE;
    dat_out = 0;
    mask_out = 1;
    while (cnt_loc)
        {
        if (dat_in & mask_inp) dat_out |= mask_out;
        mask_out <<= 1;
        mask_inp <<= 1;
        if (!mask_inp)
            {
            mask_inp = 1;
            lBase++;
            dat_in = *lBase;
            }
        cnt_loc--;
        }
    Dst[out_rc_ix] = Diapazon (dat_out);
    out_rc_ix++;
    }
}




unsigned short TSBUSRECEIVER::Diapazon (unsigned short dat)
{
float val = dat; val = val / C_SBUSCONV_DIVIDER;
return C_STARTLEV_SBUS + val;
}



void TSBUSRECEIVER::SBUSCodingArrays (unsigned char *lBase, unsigned short Src[16])
{
unsigned char mask_out = 1, cnt_loc;
unsigned char cicle = 16;
unsigned short dat_in, mask_in;
unsigned char dat_out = 0;
while (cicle)
    {
    cnt_loc = C_SBUS_RCBITS_SIZE;
    dat_in = *Src++;
    mask_in = 1;
    while (cnt_loc)
        {
        if (dat_in & mask_in) dat_out |= mask_out;
        mask_in <<= 1;
        mask_out <<= 1;
        if (!mask_out)
            {
            *lBase++ = dat_out;
            dat_out = 0;
            mask_out = 1;
            }
        cnt_loc--;
        }
    cicle--;
    }
}



