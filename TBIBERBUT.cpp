#include "TBIBERBUT.h"

#define C_SHOKINC 70

static const S_VIBRPOINT_T itmlist_prg_1[] = {
  0x40 + C_SHOKINC, 100, 0x40 + C_SHOKINC, 100, 
  0x00, 100,
  0x40 + C_SHOKINC, 50, 0x00, 100, 
  0x40 + C_SHOKINC, 50, 0x00, 100, 
  0x40 + C_SHOKINC, 50, 0x00, 100,
	0,0
};



static const S_VIBRPOINT_T itmlist_prg_2[] = {
  0x40 + C_SHOKINC, 100,
  0x00, 200,
	0,0
};



static const S_VIBRPOINT_T itmlist_prg_3[] = {
  0x60 + C_SHOKINC, 200,
	0,0
};



static const S_VIBRPOINT_T itmlist_prg_4[] = {
  0x20 + C_SHOKINC, 50,
	0x00, 200,
	0,0
};




#define C_VBREFFECT_CNT 5   // effect list
static const S_VIBRPOINT_T *effectlist[C_VBREFFECT_CNT] = {0, itmlist_prg_1, itmlist_prg_2, itmlist_prg_3, itmlist_prg_4};



TBIBERBUT::TBIBERBUT (TDRV2605 *vbr, TMAVPARAMS *prm)
{
	vibr = vbr;
	param = prm;
	butonmode[EVIBRKEY_AUTO] = param->get_value_d (EINTMVPR_MOD_AUTO).u.f;
	butonmode[EVIBRKEY_STAB] = param->get_value_d (EINTMVPR_MOD_STAB).u.f;
	butonmode[EVIBRKEY_RTL] = param->get_value_d (EINTMVPR_MOD_RTL).u.f;
	butonmode[EVIBRKEY_MIN] = param->get_value_d (EINTMVPR_MOD_MIN).u.f;
	butonmode[EVIBRKEY_MAX] = param->get_value_d (EINTMVPR_MOD_MAX).u.f;
	butonmode[EVIBRKEY_ARM] = param->get_value_d (EINTMVPR_MOD_ARM).u.f;
	butonmode[EVIBRKEY_CHUTE] = param->get_value_d (EINTMVPR_MOD_CHUTE).u.f;
	butonmode[EVIBRKEY_BANO] = param->get_value_d (EINTMVPR_MOD_BANO).u.f;
}



void TBIBERBUT::vibro (EVIBRKEY m)
{
	if (m < EVIBRKEY_ENDENUM)
		{
		uint32_t ix_effect = butonmode[m];
		if (ix_effect < C_VBREFFECT_CNT) {
			const S_VIBRPOINT_T *ef = effectlist[ix_effect];
			if (ef) {
				vibr->user_effect_set (const_cast<S_VIBRPOINT_T*>(ef));
				vibr->user_start ();
				}
			}
		}
}


