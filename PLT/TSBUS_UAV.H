#ifndef _H_SBUS_UAVCAN_CLASS_H_
#define _H_SBUS_UAVCAN_CLASS_H_



#define C_UAVCAN_SBUS_BRIDGE_SIGNATURE 											    (0x41fca0c72625e0e8ULL)	
#define C_UAVCAN_SBUS_MESSAGE_ID 20792
#define C_SBUS_ARRAY_SIZE 18
#define C_SBUS_MARGIN_TIME 3
#define C_SBUS_RCBITS_SIZE 11
#define C_SBUS_FIST_BYTE_VALUE 0xF0
#define C_SBUS_LAST_BYTE_VALUE 0

#include "main.h"
#include "TFTASKIF.h"
#include "TUSARTRXB.h"
#include "SYSBIOS.H"
#include "rutine.h"


typedef struct {
	unsigned short channels[C_SBUS_ARRAY_SIZE];
	unsigned char size;
	unsigned char flags;
} TSBUSDATA;



#define C_SBUS_PROTOCOL_ARRAY_SIZE 22

typedef struct {
	unsigned char startbyte ;
	unsigned char array[C_SBUS_PROTOCOL_ARRAY_SIZE];
	unsigned char flags;
	unsigned char endbyte;
} TSBUSPROTOCOL;


#define C_SBUS_FRAME_SIZE 25


class TSBUSRECEIVER : public TUSARTIFB, public TFFC {
	
		bool f_frame_new;
		TSBUSDATA data_new;
	
		TSBUSPROTOCOL protocol_data_frame;
		//bool f_protocol_end_forming;
		utimer_t Timer_FrameMargin;
	
		unsigned char bufrx[C_SBUS_FRAME_SIZE];
		unsigned char rx_ix;
	
		unsigned short DigChannelValue (bool val);
		void TranslateArrayTo16Bit (void *lSrc, unsigned short *lDst, unsigned char src_bits, unsigned char amount_arr);
	
		void clear_rx ();
		virtual void ISR_DataRx (unsigned char dat);
	
	public:
		TSBUSRECEIVER ();
		void Init (USART_TypeDef *p);
		virtual void Task ();
	
		bool GetData (TSBUSDATA *lDst);
	
};


#endif

