#include "TW2812SPI.h"
#include "rutine.h"

static SPI_HandleTypeDef *arr_spi[5] = {0,0,0,0,0};


#ifdef __cplusplus
 extern "C" {
#endif 
	 
static uint32_t spi_debug = 0;
	 
#ifdef SPI1
void SPI1_IRQHandler ()
{
}
#endif



#ifdef SPI2
void SPI2_IRQHandler ()
{

}
#endif


#ifdef SPI3
void SPI3_IRQHandler ()
{
}
#endif


#ifdef SPI4
void SPI4_IRQHandler ()
{
}
#endif


#ifdef SPI5
void SPI5_IRQHandler ()
{
	HAL_SPI_IRQHandler(arr_spi[4]);
}
#endif

void DMA2_Stream5_IRQHandler ()
{
	spi_debug++;
}


void DMA2_Stream6_IRQHandler ()
{
	HAL_DMA_IRQHandler(arr_spi[4]->hdmatx);
}

#ifdef __cplusplus
}
#endif




TW2812LEDS::TW2812LEDS ( S_W2812MOSI_T m, uint16_t w, uint16_t h) : c_width (w), c_height (h), c_leds_amount (w*h + 1)
{
	pinmosi = m;
	hardconstruct ();
}



TW2812LEDS::TW2812LEDS (S_W2812MOSI_T m, uint32_t cnt) : c_width (0), c_height (0), c_leds_amount (cnt + 1)
{
	pinmosi = m;
	hardconstruct ();
}



void TW2812LEDS::hardconstruct ()
{
	canva = new S_GRB_T[c_leds_amount];
	fillmem (canva, 0, c_leds_amount * sizeof(S_GRB_T));
	canva[10].u.b = 245;
	canva[12].u.r = 245;
	canva[250].u.g = 10;
	rawdmaarray = new uint8_t[raw_size ()];
	//rawdmaarray_rx = new uint8_t[raw_size ()];
	c_update_period = 50;
	f_enable = true;
	init_spi ();
}



void TW2812LEDS::enable (bool v)
{
	f_enable = v;
}



void TW2812LEDS::init_spi ()
{
	SPI_TypeDef *lSPI = hard_get_spi (pinmosi.p_ix);
  
	hard_spi_clock_enable (pinmosi.p_ix);
  __HAL_RCC_DMA2_CLK_ENABLE();
	_pin_low_init_out_pp_af ( pinmosi.af, &pinmosi.mosi , EHRTGPIOSPEED_HI);
  
	SpiHandle.Instance = lSPI;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.Mode 						 = SPI_MODE_MASTER;
  HAL_SPI_Init(&SpiHandle);

    
  /*##-3- Configure the DMA streams ##########################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = pinmosi.dmainst_tx;
  hdma_tx.Init.Channel             = pinmosi.dma_chan_tx;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
  HAL_DMA_Init(&hdma_tx);   
  
	arr_spi[4] = &SpiHandle;
	
  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA (&SpiHandle, hdmatx, hdma_tx);
    


  HAL_NVIC_SetPriority(pinmosi.itp_dma_tx, 0, 1);
  HAL_NVIC_EnableIRQ(pinmosi.itp_dma_tx);
    
  //HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);   
  //HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  
  /*##-5- Configure the NVIC for SPI #########################################*/
  HAL_NVIC_SetPriority(hard_spi_irq_type (pinmosi.p_ix), 0, 2);
  HAL_NVIC_EnableIRQ(hard_spi_irq_type (pinmosi.p_ix));
}


void TW2812LEDS::set_update_period (uint16_t ms)
{
	c_update_period = ms;
}


uint16_t TW2812LEDS::raw_size ()
{
	return c_leds_amount * 12;
}



void TW2812LEDS::start_frame ()
{
	//HAL_SPI_TransmitReceive_DMA (&SpiHandle, (uint8_t*)rawdmaarray, (uint8_t*)rawdmaarray_rx, raw_size ());
 HAL_SPI_Transmit_DMA (&SpiHandle, (uint8_t*)rawdmaarray, raw_size ());
}



void TW2812LEDS::canva_to_raw ()
{
	const uint16_t c_canvasize = c_leds_amount * sizeof(S_GRB_T);
	const uint8_t c_bit_lo = 4;
	const uint8_t c_bit_hi = 6;
	uint16_t ix = 0;
	uint8_t *src_canva = (uint8_t*)canva;
	uint8_t *dst_raw = rawdmaarray;
	uint8_t data_canv, bit_cnt, bit_ror, data_raw;
	while (ix < c_canvasize )
		{
			data_canv = *src_canva++;
			bit_cnt = 0;
			bit_ror = 0x80;
			while (bit_cnt < 8)
				{
				if (!(bit_cnt & 1))
					{
					data_raw = 0;
					if (data_canv & bit_ror) 
						{
						data_raw |= (c_bit_hi << 5);
						}
					else
						{
						data_raw |= (c_bit_lo << 5);
						}
					}
				else
					{
					if (data_canv & bit_ror) 
						{
						data_raw |= (c_bit_hi << 2);
						}
					else
						{
						data_raw |= (c_bit_lo << 2);
						}
					*dst_raw++ = data_raw;
					}
				bit_ror >>= 1;
				bit_cnt++;
				}
			ix++;
		}
}



void TW2812LEDS::task ()
{
	if (!updperiod_timer.get ())
		{
		if (HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_READY)
			{
			if (f_enable)
				{
				f_need_update = true;
				if (f_need_update)
					{
					canva_to_raw ();
					start_frame ();
					f_need_update = false;
					}
				}
			}
		updperiod_timer.set (c_update_period);
		}
}


