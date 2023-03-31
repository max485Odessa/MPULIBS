#include "KTY81220.H"

/*
      Метод измерения температуры:
  1. Определяем опорное напряжение ADC.
  2. Определяем напряжения подаваемое на делитель (константный резистор + KTY81220)
  3. Определяем ток в цепи делителя по падению напряжения на константном резисторе, определяя разницу напряжения между напряжением на делителе и средней точкой.
  4. Зная ток в цепи делителя и напряжение на термодатчике определяем сопротивление термодатчика
  5. Зная сопротивление термодатчика - по таблице сопротивлений находим его температуру
*/


static const unsigned short KTY81_210Resistance[C_KTY_TABLE_AMOUNT] = {980,1030,1135,1247,1367,1495,1630,1772,1922,2000,2080,2245,2417,2597,2785,2980,3182,3392,3607,3817,3915,4008,4166,4280};
static const short KTY81_210Temperature[C_KTY_TABLE_AMOUNT] = {-55,-50,-40,-30,-20,-10,0,10,20,25,30,40,50,60,70,80,90,100,110,120,125,130,140,150};	
	
static const unsigned short KTY81_220Resistance[C_KTY_TABLE_AMOUNT] = {990,1040,1146,1260,1381,1510,1646,1790,1941,2020,2100,2267,2441,2623,2812,3009,3214,3426,3643,3855,3955,4048,4208,4323};
static const short KTY81_220Temperature[C_KTY_TABLE_AMOUNT] = {-55,-50,-40,-30,-20,-10,0,10,20,25,30,40,50,60,70,80,90,100,110,120,125,130,140,150};
	
static const unsigned short KTY81_110Resistance[C_KTY_TABLE_AMOUNT] = {490,515,567,624,684,747,815,886,961,1000,1040,1122,1209,1299,1392,1490,1591,1696,1805,1915,1970,2023,2124,2211};
static const short KTY81_110Temperature[C_KTY_TABLE_AMOUNT] = {-55,-50,-40,-30,-20,-10,0,10,20,25,30,40,50,60,70,80,90,100,110,120,125,130,140,150};
	
//static const unsigned short KTY81_120Resistance[C_KTY_TABLE_AMOUNT] = {490,515,567,624,684,747,815,886,961,1000,1040,1122,1209,1299,1392,1490,1591,1696,1805,1915,1970,2023,2124,2211};
//static const short KTY81_120Temperature[C_KTY_TABLE_AMOUNT] = {-55,-50,-40,-30,-20,-10,0,10,20,25,30,40,50,60,70,80,90,100,110,120,125,130,140,150};


TKTY81::TKTY81 ()
{
	char ix = 0;
	while (ix < EKTY81_ENDENUM)
		{
		ktyarray[ix].c_resistor = C_TEMPERATURE_RESISTOR_OM;
		ktyarray[ix].meas = 0;
		ktyarray[ix].temperature = 0;
		ktyarray[ix].vref = 3.0;
		ktyarray[ix].f_changes_input = false;
		ix++;
		}
	ktyarray[EKTY81_1].type = EKTY81_TYPE_120;
	ktyarray[EKTY81_2].type = EKTY81_TYPE_120;			// EKTY81_TYPE_110
		
	time_out_temp = 0;
	SYSBIOS::ADD_TIMER_SYS (&time_out_temp);

}



float TKTY81::GetTemperature (EKTY81 ix)
{
return ktyarray[ix].temperature;	
}



void TKTY81::Task ()
{
	if (!time_out_temp)
		{
		// для оптимизации, за один заход вычисляется один датчик
		static char ix = EKTY81_1;
		if (ix >= EKTY81_ENDENUM) ix = EKTY81_1;
			
		if (ktyarray[ix].f_changes_input)
			{
			float curent = (ktyarray[ix].vref - ktyarray[ix].meas) / ktyarray[ix].c_resistor;   // ток в измерительной цепи пары резисторов
			float resist = ktyarray[ix].meas / curent; 		// текущее сопротивление терморезистора
			unsigned short ResistOhm = (unsigned short)resist;
			long rv = ConvertKTYData (ktyarray[ix].type, ResistOhm);
			ktyarray[ix].temperature = rv;
			ktyarray[ix].f_changes_input = false;
			}
		ix++;
		time_out_temp = C_KTY_MEASURE_PERIOD;
		}
}



void TKTY81::SetConstantResistor (EKTY81 ix, float val)
{
	if (ktyarray[ix].c_resistor != val)
		{
		ktyarray[ix].c_resistor = val;
		ktyarray[ix].f_changes_input = true;
		}
}



void TKTY81::SetRawData (EKTY81 ix, float cur_ref, float meas_volt)
{
if (ktyarray[ix].vref != cur_ref)
	{
	ktyarray[ix].vref = cur_ref + 0.048;
	ktyarray[ix].f_changes_input = true;
	}
if (ktyarray[ix].meas != meas_volt)
	{
	ktyarray[ix].meas = meas_volt;
	ktyarray[ix].f_changes_input = true;
	}
}



long TKTY81::ConvertKTYData (EKTY81_TYPE tp, unsigned short resistors)
{
long rv = Constant_Code_TemperatureError_P;
long indxs = 0;
unsigned long El1 = 0,El2 = 0;
unsigned short *ResTable = 0;
short *TempTable = 0;
switch (tp)
	{
	case EKTY81_TYPE_220:
		{
		TempTable = (short*)KTY81_220Temperature;
		ResTable = (unsigned short *)KTY81_220Resistance;
		break;
		}
	case EKTY81_TYPE_210:
		{
		TempTable = (short*)KTY81_210Temperature;
		ResTable = (unsigned short *)KTY81_210Resistance;
		break;
		}
	case EKTY81_TYPE_110:
		{
		TempTable = (short*)KTY81_110Temperature;
		ResTable = (unsigned short *)KTY81_110Resistance;
		break;
		}
	case EKTY81_TYPE_120:
		{
		TempTable = (short*)KTY81_110Temperature;
		ResTable = (unsigned short *)KTY81_110Resistance;
		break;
		}
	default:
		{
		
		}
	}
	
if (resistors >= ResTable[0])
	{
	if (resistors <= ResTable[C_KTY_TABLE_AMOUNT-1])
			{
			while (indxs < (C_KTY_TABLE_AMOUNT-1))
				{
				if (resistors >= ResTable[indxs] && resistors < ResTable[(indxs+1)])
								{
								El1 = ResTable[indxs];
								El2 = ResTable[(indxs+1)];
								break;
								}
				indxs++;
				}
			if (El1 && El2)
				{
				long curT = TempTable[indxs];
				long nxtT = TempTable[(indxs+1)];
				long diapT = nxtT - curT;
				long d2,d1 = El2 - El1;
				d1 = (d1*1000) / diapT;
				d2 = ((long)resistors - El1)*1000;
				curT = curT + (d2 / d1);
				rv = curT;
				}
			}
		else
			{
			rv = Constant_Code_TemperatureError_P;
			}
	 }
else
  {
  rv = Constant_Code_TemperatureError_M;
  }
return rv;
}


