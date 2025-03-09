#ifndef __H_I2C_RM3100DROTEK_H__
#define __H_I2C_RM3100DROTEK_H__



#include "sysbios.hpp"
#include "dispatchiface.hpp"
#include "rutine.hpp"
//#include "mag3axis_class.hpp"

enum ESWRM31 {ESWRM31_INIT = 0, ESWRM31_GETREVID = 1, ESWRM31_READ = 2, ESWRM31_ERROR = 3, ESWRM31_COMPLETE = 4};



// RM3100 Register definition
#define C_RM_POLL 		0x00
#define C_RM_CCM 			0x01
#define C_RM_CCX 			0x04
#define C_RM_CCY 			0x06
#define C_RM_CCZ 			0x08
#define C_RM_TMRC 		0x0B
#define C_RM_MX 			0x24
#define C_RM_MY 			0x27
#define C_RM_MZ 			0x2A
#define C_RM_BIST 		0x33
#define C_RM_STATUS 	0x34
#define C_RM_HSHAKE 	0x35
#define C_RM_REVID 		0x36



typedef struct {
		long X;
		long Y;
		long Z;
} TRAWRM3100;


typedef struct {
	float X;
	float Y;
	float Z;
} GAUSVALUES;



class TRM3100DR : public TI2CTRDIFACE {
	private:
		//const unsigned char C_SLAVEADRESS;
		const unsigned short C_MEASCYCLE;
		const unsigned short C_PERMIT_AMOUNT;
	
		utimer_t RDYTimeout;
		TRAWRM3100 raw_xyz;
		GAUSVALUES meass_xyz;
	
		float RawToGauss (long rawval);
		float RawToTesla (long rawval);
		bool f_to_gauss;
		
		bool SetPeriodMeas (TI2CBUS *i2cobj, unsigned short cnt_local);
		bool SetContinuosMode (TI2CBUS *i2cobj);
		bool SetSampleRate (TI2CBUS *i2cobj);
		bool InitRM3100 (TI2CBUS *i2cobj);
		bool ReadRawZYZ (TI2CBUS *i2cobj, TRAWRM3100 *lDst);
		bool GetRevision (TI2CBUS *i2cobj, unsigned char &revid);
		bool GetDataStatus (TI2CBUS *i2cobj, bool &datstate);
		bool ParseData ();
		
	protected:
		bool f_new_parse_data;
		ESWRM31 Sw;
		virtual void Task (TI2CBUS *i2cobj) override;
		virtual void Start () override;
		virtual const char *GetChipName () override;
	
	public:
		TRM3100DR (uint8_t slv_adr);

		void SetMeassTypeGauss (bool c_types);
		void ClearFlagNewData ();							
		bool CheckNewData ();									
		GAUSVALUES *GetXYZ ();	
};




#endif

