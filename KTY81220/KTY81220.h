#ifndef __H_KTY8110_H_
#define __H_KTY8110_H_


#include "TFTASKIF.h"
#include "SYSBIOS.H"


#define C_TEMPERATURE_RESISTOR_OM 1100
#define Constant_Code_TemperatureError_P 127
#define Constant_Code_TemperatureError_M -100
#define C_KTY_TABLE_AMOUNT 24
#define C_KTY_MEASURE_PERIOD 1000												// для оптимизации перерасчет показаний датчика 1 раз в секунду


enum EKTY81 {EKTY81_1 = 0, EKTY81_2 = 1, EKTY81_ENDENUM = 2};															// номер датчика в системе
enum EKTY81_TYPE {EKTY81_TYPE_220 = 0, EKTY81_TYPE_110 = 1, EKTY81_TYPE_210 = 2, EKTY81_TYPE_120 = 3, EKTY81_TYPE_ENDENUM};			// тип датчика

typedef struct {
	float c_resistor;
	float vref;
	float meas;
	float temperature;
	EKTY81_TYPE type;
	bool f_changes_input;
} SKTYDATA;


class TKTY81: public TFFC {
	
	private:
		long ConvertKTYData (EKTY81_TYPE tp, unsigned short resistors);
		SKTYDATA ktyarray[EKTY81_ENDENUM];
		utimer_t time_out_temp;
		virtual void Task ();

	public:
		TKTY81 ();
	
		void SetConstantResistor (EKTY81 ix, float val);								// устанавливает номинал резистора необходимый для расчета падения напряжения
		void SetRawData (EKTY81 ix, float cur_ref, float meas_volt);		// передает: опорное напряжение и напряжение в измерительной точке
	
		float GetTemperature (EKTY81 ix);
		
};


#endif

