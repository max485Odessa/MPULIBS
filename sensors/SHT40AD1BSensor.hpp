#ifndef SHT40AD1B_H
#define SHT40AD1B_H

#define SHT40AD1B_I2C_ADDRESS  0x44

/* Includes ------------------------------------------------------------------*/
//#include "Wire.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "DISPATCHIFACE.hpp"
#include "sysbios.hpp"


/* Typedefs ------------------------------------------------------------------*/
typedef enum {
  SHT40AD1B_STATUS_OK = 0,
  SHT40AD1B_STATUS_ERROR
} SHT40AD1BStatusTypeDef;


typedef struct {
	union	{
		float a[2];
		struct {
			float hum;
			float temp;
			} s;
	} u;
} S_SHT40RESULT_TAG;




class SHT40AD1BSensor: public TI2CTRDIFACE {
	enum ETEMPSW {ETEMPSW_INIT = 0, ETEMPSW_REQ_MEASURE, ETEMPSW_READ, ETEMPSW_ERROR, ETEMPSW_COMPLETE, ETEMPSW_ENDENUM};
	enum ERDRESULTTYPE {ERDRESULTTYPE_OK = 0, ERDRESULTTYPE_BUS, ERDRESULTTYPE_CRC, ERDRESULTTYPE_ENDENUM};

  public:
    SHT40AD1BSensor(uint8_t adr, uint32_t prs_ms);

    bool GetHumidity (float &hum_value);
    bool GetTemperature (float &tmp_value);

  protected:
    bool init ();
    ERDRESULTTYPE read (S_SHT40RESULT_TAG *dst);
    bool req_measure ();

  private:
    uint8_t crc_calculate(const uint8_t *data, uint16_t count);
    uint8_t crc_check(const uint8_t *data, uint16_t count, uint8_t crc);

    const uint32_t c_period_read_ms;
    ETEMPSW sw_state;
	TI2CBUS *i2c_bus;
	virtual void Task (TI2CBUS *i2cobj) override;
	virtual void Start () override;
	virtual const char *GetChipName () override;
	SYSBIOS::Timer relax_tim;
	SYSBIOS::Timer timeoutdata_tim;
	bool f_read_is_ok;
	S_SHT40RESULT_TAG data_result;

    /* Helper classes. */
    //TwoWire *dev_i2c;
    /* Configuration */
    //uint8_t address;
};


#endif
