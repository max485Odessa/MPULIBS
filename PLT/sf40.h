#ifndef _H_SF40_DISTANCE_H_
#define _H_SF40_DISTANCE_H_



#include "TUSARTIFACE.h"
#include "resources.h"
#include "TFTASKIF.h"
#include "SYSBIOS.H"
#include "rutine.h"
//#include "TTFIFO.h"


#define PROXIMITY_SF40C_TIMEOUT_MS            200   // requests timeout after 0.2 seconds
#define PROXIMITY_SF40C_PAYLOAD_LEN_MAX       256   // maximum payload size we can accept (in some configurations sensor may send as large as 1023)
#define PROXIMITY_SF40C_COMBINE_READINGS        7   // combine this many readings together to improve efficiency
#define PROXIMITY_SF40C_HEADER                  0xAA
#define PROXIMITY_SF40C_DESIRED_OUTPUT_RATE     3
#define C_SF40TXBUF_SIZE 2048

#define C_LIDAR_POINTS_MAX 48

#pragma pack (push, 1)



typedef struct {
	unsigned char AlarmState;						// 0
	unsigned short PointsPerSecond;			// 1
	short ForwardOffset;									// 3
	short MotorVoltage;									// 5
	unsigned char RevolutionIndex;				// 7
	unsigned short PointTotal;						// 8
	unsigned short PointCount;						// 10
	unsigned short PointStartIndex;				// 12
	// ...array PointCount * int16
	unsigned short array[C_LIDAR_POINTS_MAX];
} S_SF40_DISTANCE_PAY;



typedef struct {
	unsigned char State;
} S_SF40_STATE_PAY;



typedef struct {
	unsigned long data;
} S_SF40_STREAM_PAY;



typedef struct {
	unsigned char data;						// 0 - 20010, 1 - 10005, 2 - 6670, 3 - 2001
} S_SF40_OUTRATE_PAY;



typedef struct {
	unsigned char data[2];
} S_SF40_TOKKEN_PAY;



#pragma pack (pop)


class TSF40: public TFFC, public TUSART_IFACE {
	
		unsigned char txbuf[C_SF40TXBUF_SIZE];
		unsigned long tx_add_ix;
		void push_tx (unsigned char b);
		
		bool f_is_working;	
		bool f_is_receive;
	
		//static THC595 latch_lpn;
	
    enum MessageID{
        PRODUCT_NAME = 0,
        HARDWARE_VERSION = 1,
        FIRMWARE_VERSION = 2,
        SERIAL_NUMBER = 3,
        TEXT_MESSAGE = 7,
        USER_DATA = 9,
        TOKEN = 10,
        SAVE_PARAMETERS = 12,
        RESET = 14,
        STAGE_FIRMWARE = 16,
        COMMIT_FIRMWARE = 17,
        INCOMING_VOLTAGE = 20,
        STREAM = 30,
        DISTANCE_OUTPUT = 48,
        LASER_FIRING = 50,
        TEMPERATURE = 55,
        BAUD_RATE = 90,
        DISTANCE = 105,
        MOTOR_STATE = 106,
        MOTOR_VOLTAGE = 107,
        OUTPUT_RATE = 108,
        FORWARD_OFFSET = 109,
        REVOLUTIONS = 110,
        ALARM_STATE = 111,
        ALARM1 = 112,
        ALARM2 = 113,
        ALARM3 = 114,
        ALARM4 = 115,
        ALARM5 = 116,
        ALARM6 = 117,
        ALARM7 = 118
    };
	
    enum  MotorState{
        UNKNOWN = 0,
        PREPARING_FOR_STARTUP = 1,
        WAITING_FOR_FIVE_REVS = 2,
        RUNNING_NORMALLY = 3,
        FAILED_TO_COMMUNICATE = 4
    };
    // state of sensor
    struct {
        MotorState motor_state; // motor state (1=starting-up,2=waiting for first 5 revs, 3=normal, 4=comm failure)
        uint8_t output_rate;    // output rate number (0 = 20010, 1 = 10005, 2 = 6670, 3 = 2001)
        bool streaming;         // true if distance messages are being streamed
        uint8_t token[2];       // token (supplied by sensor) required for reset
    } _sensor_state;
	
    enum ParseState {
        HEADER = 0,
        FLAGS_L,
        FLAGS_H,
        MSG_ID,
        PAYLOAD,
        CRC_L,
        CRC_H
    };
	
    struct {
        ParseState state;       // state of incoming message processing
        uint8_t flags_low;      // flags low byte
        uint8_t flags_high;     // flags high byte
        uint16_t payload_len;   // latest message payload length (1+ bytes in payload)
        uint8_t payload[PROXIMITY_SF40C_PAYLOAD_LEN_MAX];   // payload
        MessageID msgid;        // latest message's message id
        uint16_t payload_recv;  // number of message's payload bytes received so far
        uint8_t crc_low;        // crc low byte
        uint8_t crc_high;       // crc high byte
        uint16_t crc_expected;  // latest message's expected crc
    } _msg;

		virtual void ISR_TxEnd_cb ();
		virtual void ISR_DataRx (unsigned char dat);
		
    uint32_t buff_to_uint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) const;
    uint16_t buff_to_uint16(uint8_t b0, uint8_t b1) const;
		
		void parse_byte(uint8_t b);
		bool parse_req ();			// если true - ошибок нет
		void send_message (MessageID msgid, bool write, const uint8_t *payload, uint16_t payload_len);
		
		utimer_t _last_reply_ms;
		utimer_t tx_relax_period;
		
		unsigned short lidar_points[C_LIDAR_POINTS_MAX];
		void send_distance_array ();
		
	public:
		TSF40 ();
		
		virtual void Task ();
		void Init (unsigned long speedd);
		//static THC595 *latch_lpn;
	
		void Angle_set (unsigned short angl, unsigned short dimns);
		void Angle_clear ();
		void SetLidarPoints (unsigned char pcnt);
		
};



#endif
