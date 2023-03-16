#ifndef _H_SBUS_UAVCAN_CLASS_H_
#define _H_SBUS_UAVCAN_CLASS_H_



#define C_UAVCAN_SBUS_BRIDGE_SIGNATURE 											    (0x41fca0c72625e0e8ULL)	
#define C_UAVCAN_SBUS_MESSAGE_ID 20792

#define C_SBUS_ARRAY_SIZE 18
#define C_SBUS_RCBITS_SIZE 11
#define C_SBUS_PROTOCOL_ARRAY_SIZE 22
#define C_SBUS_FIST_BYTE_VALUE ((unsigned char)0x0F)
#define C_SBUS_LAST_BYTE_VALUE ((unsigned char)0)
#define C_SBUS_FRAME_SIZE 25


// константы для преобразования в требуемый диапазон величин
#define C_RAWSBUS_START 184
#define C_RAWSBUS_STOP 1820
#define C_RAWSBUS_DIAP (C_RAWSBUS_STOP-C_RAWSBUS_START)
#define C_DIAPSBUS 1027.0
#define C_SBUSCONV_DIVIDER (C_RAWSBUS_DIAP/C_DIAPSBUS)  // ~1.5929
#define C_STARTLEV_SBUS 875


//#include "app.h"
#include "TFTASKIF.h"
#include "TUSARTIFACE.h"
#include "SYSBIOS.H"
#include "rutine.h"




typedef struct {
	unsigned short channels[C_SBUS_ARRAY_SIZE];
	unsigned char flags;
} TSBUSDATA;


typedef struct {
	unsigned char startbyte ;
	unsigned char array[C_SBUS_PROTOCOL_ARRAY_SIZE];
	unsigned char flags;
	unsigned char endbyte;
} TSBUSPROTOCOL;



typedef struct {
    unsigned short rc[16];
} SBUS16BITARRAY;




class TSBUSIFACE {
	protected:
		bool f_new_frame_isr;
		
	public:
		void UnBlockAfter_recv ();
		virtual TSBUSDATA *GetData () = 0;
		virtual void Init () = 0;
};




class TSBUSRECEIVER : public TUSART_IFACE, public TFFC, public TSBUSIFACE {
	

		TSBUSDATA data_new;
	
		bool f_new_frame_check;
		TSBUSPROTOCOL protocol_data_frame;
		unsigned char bufrx[C_SBUS_FRAME_SIZE];
		unsigned char rx_ix;
	
		unsigned short DigChannelValue (bool val);
		void TranslateArrayTo16Bit (void *lSrc, unsigned short *lDst);
	
		void clear_rx ();
		virtual void ISR_DataRx (unsigned char dat);
		void GenSBUS_frames (SBUS16BITARRAY *inp, TSBUSPROTOCOL &dst, bool rc_ok, bool fail_safe);
		void sbus_format_frame (unsigned short *channels,  unsigned char *buffer);
	
		SBUS16BITARRAY arr16;
		
		unsigned short frame_marg_timeout;
		
		void SBUSDecodingArrays (unsigned char *lBase, unsigned short Dst[16]);
		void SBUSCodingArrays (unsigned char *lBase, unsigned short Src[16]);
		
		unsigned short Diapazon (unsigned short d);
	
	public:
		TSBUSRECEIVER ();
		virtual void Init ();
		virtual void Task ();
		void Isr_100mks ();
	
		virtual TSBUSDATA *GetData ();
	
	
};


#endif

