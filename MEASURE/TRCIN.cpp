#include "TRCIN.h"
#include "definitions.h"





TRCIN *TRCIN::singlobj = 0;



TRCIN::TRCIN ()
{
	RCSmooth[ROLL] = 0; RCSmooth[PITCH] = 0; RCSmooth[YAW] = 0;
}



TRCIN *TRCIN::obj ()
{
	if (!singlobj) singlobj = new TRCIN ();
	return singlobj;
}



IRCIN *TRCIN::obj_usr ()
{
return obj ();
}


//static S_MOTOR_ANGLES_T anglemotors;

/*
bool TRCIN::Get_Raw360_Angles (S_MOTOR_ANGLES_T *dst)
{
	bool rv = false;
	float mult = (1.0F / 65535) * 359.9F;
	if (dst)
		{
		dst->yaw = mult * hard_arr_vals[YAW];
		dst->roll = mult * hard_arr_vals[ROLL];
		dst->pitch = mult * hard_arr_vals[PITCH];
		anglemotors = *dst;
		rv = true;
		}
	return rv;
}
*/






long TRCIN::GetAUX3 ()
{
	return 0;
}



long TRCIN::GetAUX4 ()
{
	return 0;
}



long TRCIN::GetAUX2 ()
{
	return 0;
}



void TRCIN::init ()
{

}



// output pange Step:  0.0F - 1.0F
void TRCIN::Get_RC_Step (float *Step)
{

    int aux3 = GetAUX3(); //PITCH
    int aux2 = GetAUX2(); //ROLL
    int aux4 = GetAUX4(); //YAW
		// value range RCSmooth: -500.0 до 500.0
    // Pitch
    if (aux3 != 0) //check there is a rc input
    {
        RCSmooth[PITCH] = ((RCSmooth[PITCH] * 199) + (aux3 - RC_CENTER_VAL)) / 200;

        if (RCSmooth[PITCH] > DEAD_ZONE || RCSmooth[PITCH] < -DEAD_ZONE)
        {
            Step[PITCH] = RCSmooth[PITCH] * RC2STEP;
        }
        else
        {
            Step[PITCH] = 0;
        }
    }
    else
    {
        Step[PITCH] = 0;
    }

    // Roll
    if (aux2 != 0) //check there is a rc input
    {
        RCSmooth[ROLL] = ((RCSmooth[ROLL] * 199) + (aux2 - RC_CENTER_VAL)) / 200;

        if (RCSmooth[ROLL] > DEAD_ZONE || RCSmooth[ROLL] < -DEAD_ZONE)
        {
            Step[ROLL] = RCSmooth[ROLL] * RC2STEP;
        }
        else
        {
            Step[ROLL] = 0;
        }
    }
    else
    {
        Step[ROLL] = 0;
    }

    // YAW
    if (aux4 != 0) //check there is a rc input
    {
        RCSmooth[YAW] = ((RCSmooth[YAW] * 199) + (aux4 - RC_CENTER_VAL)) / 200;

        if (RCSmooth[YAW] > DEAD_ZONE || RCSmooth[YAW] < -DEAD_ZONE)
        {
            Step[YAW] = RCSmooth[YAW] * RC2STEP;
        }
        else
        {
            Step[YAW] = 0;
        }
    }
    else
    {
        Step[YAW] = 0;
    }
}



void TRCIN::Task ()
{
}





