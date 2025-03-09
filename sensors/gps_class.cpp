#include "gps_class.hpp"
#include "main.h"
#include <math.h>

//TUSART_IFACE* TUSART_IFACE::lFS_PUSART[EUSARTINDX_MAX] = {0,0,0};

 const char *ltxt_gngga = "GNGGA";
 const char *ltxt_gpgga = "GPGGA";
 const char *ltxt_gnrmc = "GNRMC";
 const char *ltxt_gprmc = "GPRMC";
 const char *ltxt_gngsa = "GNGSA";
 const char *ltxt_gpgsa = "GPGSA";
 const char *ltxt_gngns = "GNGNS";
 const char *ltxt_gpgns = "GPGNS";

uint8_t IGPS::cur_sat_amount = 0;
uint8_t IGPS::gpsDmode = 0;
volatile utimer_t IGPS::Timer_RTK_implement = 0;



/*
static void wgsllh2ecef (double lat, double lon, double alt , ECEF_POINT &point)
{
	lat *= D2R;
	lon *= D2R;
  double d = WGS84_E * sin(lat);
  double N = WGS84_A / sqrt(1.0 - d * d);

  point.x = (N + alt) * cos(lat) * cos(lon);
  point.y = (N + alt) * cos(lat) * sin(lon);
  //point.z = (((1 - WGS84_E * WGS84_E) * N + alt) * lat);

  point.z = ((1 - WGS84_E * WGS84_E) * N + alt) * sin(lat);
}



static void wgsecef2llh(ECEF_POINT &ecef, WGS84POINT &llh)
{
  const double p = sqrt(ecef.x * ecef.x + ecef.y * ecef.y);

  if (p != 0) {
    llh.longitude = atan2(ecef.y, ecef.x);
  } else {
    llh.longitude = 0;
  }


  if (p < WGS84_A * 1e-16) {
    llh.latitude = copysign(M_PI_2, ecef.z);
    llh.altitude = fabs(ecef.z) - WGS84_B;

    llh.latitude *= R2D;
    llh.longitude *= R2D;
    return;
  }

  const double P = p / WGS84_A;
  const double e_c = sqrt(1. - WGS84_E * WGS84_E);
  const double Z = fabs(ecef.z) * e_c / WGS84_A;

  double S = Z;
  double C = e_c * P;


  double prev_C = -1;
  double prev_S = -1;

  double A_n, B_n, D_n, F_n;


  for (int i = 0; i < 10; i++) {

    A_n = sqrt(S * S + C * C);
    D_n = Z * A_n * A_n * A_n + WGS84_E * WGS84_E * S * S * S;
    F_n = P * A_n * A_n * A_n - WGS84_E * WGS84_E * C * C * C;
    B_n = 1.5 * WGS84_E * S * C * C * (A_n * (P * S - Z * C) - WGS84_E * S * C);


    S = D_n * F_n - B_n * S;
    C = F_n * F_n - B_n * C;



    if (S > C) {
      C = C / S;
      S = 1;
    } else {
      S = S / C;
      C = 1;
    }

    if (fabs(S - prev_S) < 1e-16 && fabs(C - prev_C) < 1e-16) {
      break;
    }
    prev_S = S;
    prev_C = C;
  }

  A_n = sqrt(S * S + C * C);
  llh.latitude = copysign(1.0, ecef.z) * atan(S / (e_c * C));
  llh.altitude = (p * e_c * C + fabs(ecef.z) * S - WGS84_A * e_c * A_n) / sqrt(e_c * e_c * C * C + S * S);


  llh.latitude *= R2D;
  llh.longitude *= R2D;
}
*/


/*
void lla2ecef (long double lat, long double lon, long double alt, ECEF_POINT &point)
{
    // WGS84 ellipsoid constants:
    const long double a = 6378137;            // (m) at the equator
    const long double e = 0.081819190842622;  // eccentricity

    // intermediate calculation
    // (prime vertical radius of curvature)
    long double N = a / sqrt(1 - (e * e) * (sin(lat) * sin(lat)));

    // results:
    point.x = (N + alt) * cos(lat) * cos(lon);
    point.y = (N + alt) * cos(lat) * sin(lon);
    point.z = ((1 - (e * e)) * N + alt) * sin(lat);
}



void ecef2lla_helper(long double x, long double y, long double z, WGS84POINT &coords)
{

    // WGS84 ellipsoid constants:
    const long double a = 6378137;            // (m) at the equator
    const long double e = 0.081819190842622;  // eccentricity

    // calculations:
    long double b   = sqrt( (a * a) * (1 - e * e ));
    long double ep  = sqrt( (a * a - b * b ) / (b * b));
    long double p   = sqrt( x * x + y * y );
    long double th  = atan2(a * z, b * p);
    long double lon = atan2(y, x);
    long double lat = atan2( (z + ep * ep * b * sin(th) * sin(th) * sin(th)), (p - e * e * a * cos(th) * cos(th) * cos(th)) );
    long double N   = a / sqrt( 1 - e * e * sin(lat) * sin(lat));
    long double alt = p / cos(lat) - N;

    // return lon in range [0,2*pi)
    lon = lon - (M_PI * 2) * floor( lon / (M_PI * 2) );

    coords.latitude = lat;
    coords.longitude = lon;
    coords.altitude = alt;
}
*/


static const unsigned long crc24q[256] = {
    0x00000000, 0x01864CFB, 0x028AD50D, 0x030C99F6,
    0x0493E6E1, 0x0515AA1A, 0x061933EC, 0x079F7F17,
    0x08A18139, 0x0927CDC2, 0x0A2B5434, 0x0BAD18CF,
    0x0C3267D8, 0x0DB42B23, 0x0EB8B2D5, 0x0F3EFE2E,
    0x10C54E89, 0x11430272, 0x124F9B84, 0x13C9D77F,
    0x1456A868, 0x15D0E493, 0x16DC7D65, 0x175A319E,
    0x1864CFB0, 0x19E2834B, 0x1AEE1ABD, 0x1B685646,
    0x1CF72951, 0x1D7165AA, 0x1E7DFC5C, 0x1FFBB0A7,
    0x200CD1E9, 0x218A9D12, 0x228604E4, 0x2300481F,
    0x249F3708, 0x25197BF3, 0x2615E205, 0x2793AEFE,
    0x28AD50D0, 0x292B1C2B, 0x2A2785DD, 0x2BA1C926,
    0x2C3EB631, 0x2DB8FACA, 0x2EB4633C, 0x2F322FC7,
    0x30C99F60, 0x314FD39B, 0x32434A6D, 0x33C50696,
    0x345A7981, 0x35DC357A, 0x36D0AC8C, 0x3756E077,
    0x38681E59, 0x39EE52A2, 0x3AE2CB54, 0x3B6487AF,
    0x3CFBF8B8, 0x3D7DB443, 0x3E712DB5, 0x3FF7614E,
    0x4019A3D2, 0x419FEF29, 0x429376DF, 0x43153A24,
    0x448A4533, 0x450C09C8, 0x4600903E, 0x4786DCC5,
    0x48B822EB, 0x493E6E10, 0x4A32F7E6, 0x4BB4BB1D,
    0x4C2BC40A, 0x4DAD88F1, 0x4EA11107, 0x4F275DFC,
    0x50DCED5B, 0x515AA1A0, 0x52563856, 0x53D074AD,
    0x544F0BBA, 0x55C94741, 0x56C5DEB7, 0x5743924C,
    0x587D6C62, 0x59FB2099, 0x5AF7B96F, 0x5B71F594,
    0x5CEE8A83, 0x5D68C678, 0x5E645F8E, 0x5FE21375,
    0x6015723B, 0x61933EC0, 0x629FA736, 0x6319EBCD,
    0x648694DA, 0x6500D821, 0x660C41D7, 0x678A0D2C,
    0x68B4F302, 0x6932BFF9, 0x6A3E260F, 0x6BB86AF4,
    0x6C2715E3, 0x6DA15918, 0x6EADC0EE, 0x6F2B8C15,
    0x70D03CB2, 0x71567049, 0x725AE9BF, 0x73DCA544,
    0x7443DA53, 0x75C596A8, 0x76C90F5E, 0x774F43A5,
    0x7871BD8B, 0x79F7F170, 0x7AFB6886, 0x7B7D247D,
    0x7CE25B6A, 0x7D641791, 0x7E688E67, 0x7FEEC29C,
    0x803347A4, 0x81B50B5F, 0x82B992A9, 0x833FDE52,
    0x84A0A145, 0x8526EDBE, 0x862A7448, 0x87AC38B3,
    0x8892C69D, 0x89148A66, 0x8A181390, 0x8B9E5F6B,
    0x8C01207C, 0x8D876C87, 0x8E8BF571, 0x8F0DB98A,
    0x90F6092D, 0x917045D6, 0x927CDC20, 0x93FA90DB,
    0x9465EFCC, 0x95E3A337, 0x96EF3AC1, 0x9769763A,
    0x98578814, 0x99D1C4EF, 0x9ADD5D19, 0x9B5B11E2,
    0x9CC46EF5, 0x9D42220E, 0x9E4EBBF8, 0x9FC8F703,
    0xA03F964D, 0xA1B9DAB6, 0xA2B54340, 0xA3330FBB,
    0xA4AC70AC, 0xA52A3C57, 0xA626A5A1, 0xA7A0E95A,
    0xA89E1774, 0xA9185B8F, 0xAA14C279, 0xAB928E82,
    0xAC0DF195, 0xAD8BBD6E, 0xAE872498, 0xAF016863,
    0xB0FAD8C4, 0xB17C943F, 0xB2700DC9, 0xB3F64132,
    0xB4693E25, 0xB5EF72DE, 0xB6E3EB28, 0xB765A7D3,
    0xB85B59FD, 0xB9DD1506, 0xBAD18CF0, 0xBB57C00B,
    0xBCC8BF1C, 0xBD4EF3E7, 0xBE426A11, 0xBFC426EA,
    0xC02AE476, 0xC1ACA88D, 0xC2A0317B, 0xC3267D80,
    0xC4B90297, 0xC53F4E6C, 0xC633D79A, 0xC7B59B61,
    0xC88B654F, 0xC90D29B4, 0xCA01B042, 0xCB87FCB9,
    0xCC1883AE, 0xCD9ECF55, 0xCE9256A3, 0xCF141A58,
    0xD0EFAAFF, 0xD169E604, 0xD2657FF2, 0xD3E33309,
    0xD47C4C1E, 0xD5FA00E5, 0xD6F69913, 0xD770D5E8,
    0xD84E2BC6, 0xD9C8673D, 0xDAC4FECB, 0xDB42B230,
    0xDCDDCD27, 0xDD5B81DC, 0xDE57182A, 0xDFD154D1,
    0xE026359F, 0xE1A07964, 0xE2ACE092, 0xE32AAC69,
    0xE4B5D37E, 0xE5339F85, 0xE63F0673, 0xE7B94A88,
    0xE887B4A6, 0xE901F85D, 0xEA0D61AB, 0xEB8B2D50,
    0xEC145247, 0xED921EBC, 0xEE9E874A, 0xEF18CBB1,
    0xF0E37B16, 0xF16537ED, 0xF269AE1B, 0xF3EFE2E0,
    0xF4709DF7, 0xF5F6D10C, 0xF6FA48FA, 0xF77C0401,
    0xF842FA2F, 0xF9C4B6D4, 0xFAC82F22, 0xFB4E63D9,
    0xFCD11CCE, 0xFD575035, 0xFE5BC9C3, 0xFFDD8538,
};



static const unsigned long aulCrcTable[256] =
{
0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL, 0x706af48fUL,
0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL, 0xe0d5e91eUL, 0x97d2d988UL,
0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL, 0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL,
0xf3b97148UL, 0x84be41deUL, 0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL,
0x136c9856UL, 0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL, 0xa2677172UL,
0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL, 0x35b5a8faUL, 0x42b2986cUL,
0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL, 0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL,
0x26d930acUL, 0x51de003aUL, 0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL,
0xcfba9599UL, 0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL, 0x01db7106UL,
0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL, 0x9fbfe4a5UL, 0xe8b8d433UL,
0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL, 0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL,
0x91646c97UL, 0xe6635c01UL, 0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL,
0x6c0695edUL, 0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL, 0xfbd44c65UL,
0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL, 0x4adfa541UL, 0x3dd895d7UL,
0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL, 0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL,
0x44042d73UL, 0x33031de5UL, 0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL,
0xbe0b1010UL, 0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL, 0x2eb40d81UL,
0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL, 0x03b6e20cUL, 0x74b1d29aUL,
0xead54739UL, 0x9dd277afUL, 0x04db2615UL, 0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL,
0x0d6d6a3eUL, 0x7a6a5aa8UL, 0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL,
0xf00f9344UL, 0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL, 0x67dd4accUL,
0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL, 0xd6d6a3e8UL, 0xa1d1937eUL,
0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL, 0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL,
0xd80d2bdaUL, 0xaf0a1b4cUL, 0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL,
0x316e8eefUL, 0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL, 0xb2bd0b28UL,
0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL, 0x2cd99e8bUL, 0x5bdeae1dUL,
0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL, 0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL,
0x72076785UL, 0x05005713UL, 0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL,
0x92d28e9bUL, 0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL, 0x18b74777UL,
0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL, 0x8f659effUL, 0xf862ae69UL,
0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL, 0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL,
0xa7672661UL, 0xd06016f7UL, 0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL,
0x40df0b66UL, 0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL, 0xcdd70693UL,
0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL, 0x5d681b02UL, 0x2a6f2b94UL,
0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL, 0x2d02ef8dUL
};





IGPS::IGPS (EUSARTSPD si, uint32_t asz, TSERIALUSR *u_obj, S_GPIOPIN *p_pwr) : c_work_speed_ix(si), c_rx_buf_alloc_size(asz)
{
	pin_power = p_pwr;
	usartobj = u_obj;
	rxdatabuf = new uint8_t[c_rx_buf_alloc_size];
	push_ix = 0;
	loc_cnt_messages = 0;
	//f_new_gps_data = false;
	//f_new_aux_data = false;
	memset (&GPSAccData, 0, sizeof(GPSAccData));
	Timer_RTK_implement = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_RTK_implement);
	
	GPSAccData.UTC_time.hdr.valid_timeout = 0;
	GPSAccData.UTD_date.hdr.valid_timeout = 0;
	GPSAccData.Pos_lat.hdr.valid_timeout = 0;
	GPSAccData.Pos_lon.hdr.valid_timeout = 0;
	GPSAccData.pos_pole_NS.hdr.valid_timeout = 0;
	GPSAccData.pos_pole_EW.hdr.valid_timeout = 0;
	GPSAccData.Altitud.hdr.valid_timeout = 0;
	GPSAccData.MagVar.hdr.valid_timeout = 0;
	GPSAccData.TrCourse.hdr.valid_timeout = 0;
	GPSAccData.SpeedData.hdr.valid_timeout = 0;
	GPSAccData.Difelipsheight.hdr.valid_timeout = 0;
	GPSAccData.mag_EW.hdr.valid_timeout = 0;
	GPSAccData.modeFIX2.hdr.valid_timeout = 0;
	GPSAccData.modequality_gga.hdr.valid_timeout = 0;
	GPSAccData.mode2d3d_gsa.hdr.valid_timeout = 0;
	GPSAccData.satelitevieww.hdr.valid_timeout = 0;
	GPSAccData.sateliteused.hdr.valid_timeout = 0;
	GPSAccData.rtk_status.hdr.valid_timeout = 0;
	GPSAccData.dgps_submode.hdr.valid_timeout = 0;
	GPSAccData.covs.hdr.valid_timeout = 0;
	GPSAccData.PDOP.hdr.valid_timeout = 0;
	GPSAccData.VDOP.hdr.valid_timeout = 0;
	GPSAccData.HDOP.hdr.valid_timeout = 0;
	GPSAccData.EDOP.hdr.valid_timeout = 0;
	GPSAccData.NDOP.hdr.valid_timeout = 0;
	GPSAccData.TDOP.hdr.valid_timeout = 0;
	GPSAccData.GDOP.hdr.valid_timeout = 0;
	
	RXTraficTimeout = 0;
	Timer_req_DOP = 0;
	Timer_RATE = 0;
	OtherTimer = 0;
	RMC_contrl_Timer = 0;
	GGA_contrl_Timer = 0;
	GSA_contrl_Timer = 0;
		
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.UTC_time.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.UTD_date.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.Pos_lat.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.Pos_lon.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.pos_pole_NS.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.pos_pole_EW.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.Altitud.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.MagVar.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.TrCourse.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.SpeedData.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.Difelipsheight.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.mag_EW.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.modeFIX2.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.modequality_gga.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.mode2d3d_gsa.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.satelitevieww.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.sateliteused.hdr.valid_timeout);
	//SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.fix_status.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.rtk_status.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.dgps_submode.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.covs.hdr.valid_timeout);
	
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.PDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.VDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.HDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.EDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.NDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.TDOP.hdr.valid_timeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GPSAccData.GDOP.hdr.valid_timeout);
	
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&RXTraficTimeout);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_req_DOP);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_RATE);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&OtherTimer);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&RMC_contrl_Timer);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GGA_contrl_Timer);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&GSA_contrl_Timer);

	clear_covariances ();
	OtherTimer = C_SPEEDSET_TIMEOUT;
	start_to_power_wait ();
}




static unsigned long CalculateCRC32 (uint8_t *szBuf,  int iSize)
{
int iIndex;
unsigned long ulCRC = 0;
for (iIndex=0; iIndex<iSize; iIndex++)
	{
	ulCRC = aulCrcTable[(ulCRC ^ szBuf[iIndex]) & 0xff] ^ (ulCRC >> 8);
	}
return ulCRC;
}


bool IGPS::CalculateUM_CRC (unsigned char *lSrc, unsigned short sz, S_UM_CRC32_T *lDstCrc)
{
	bool rv = false;
	unsigned long ulCRC = CalculateCRC32 (lSrc,  sz);
	if (lDstCrc) lDstCrc->crc = ulCRC;
	return rv;
}



void IGPS::to_detect_speed ()
{
	usart_speed_cnt = EUSARTSPD_MAX;
	SWVal = EGPSSW_SPEED_SET;
}




bool IGPS::RawTransmit (void *s, uint32_t sz)
{
	bool rv = false;
	if (usartobj) rv = usartobj->Tx(s, sz);
	return rv;
}



void IGPS::gps_uart_speed_set (EUSARTSPD vsp)
{
	if (usartobj && vsp < EUSARTSPD_MAX) usartobj->SetSpeed(C_USARTSPEED_INDX[vsp]);
}




TCANARDUBLOX::TCANARDUBLOX (EUSARTSPD si, uint32_t asz, TSERIALUSR *obj, S_GPIOPIN *p_pwr) : IGPS(si, asz, obj, p_pwr)
{

	usart_speed_ix = EUSARTSPD_38400;
	to_detect_speed ();
	
	UBXReqTimet_PVT = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&UBXReqTimet_PVT);

	cur_sat_amount = 0;
	gpsDmode = 0;
	
}



bool IGPS::CalculateUBX_CRC (unsigned char *lSrc, unsigned short sz, TUBXCRC *lDstCrc)
{
	bool rv = false;
	unsigned char CK_A = 0;
	unsigned char CK_B = 0;
	unsigned char *lEnd = rxdatabuf + c_rx_buf_alloc_size - 1;
	if (lSrc)
		{
		while (sz && lEnd >= lSrc)
			{
			CK_A = CK_A + lSrc[0];
			CK_B = CK_B + CK_A;
			lSrc++;
			sz--;
			}
		if (lEnd >= lSrc) rv = true;
		}
		
	if (lDstCrc)
		{
		lDstCrc->CK_A = CK_A;
		lDstCrc->CK_B = CK_B;
		}
return rv;		
}



void TCANARDUBLOX::WriteUBX_CRC (HDRUBX *ldst)
{
	if (ldst)
		{
		unsigned short cur_sz = sizeof(HDRUBX) - 2;		// 2 ����� ���������
		cur_sz += ldst->Len;
		TUBXCRC *lDCrc = (TUBXCRC*)(((char*)ldst) + sizeof(HDRUBX) + ldst->Len);
		CalculateUBX_CRC ((unsigned char *)&ldst->Class, cur_sz, lDCrc);
		}
}



void TCANARDUBLOX::WriteUBXSignature (HDRUBX *ldst)
{
	ldst->Preamble_A = CUBXPREX_A;
	ldst->Preamble_B = CUBXPREX_B;
}



unsigned short TCANARDUBLOX::GetUBXFrameSize (HDRUBX *lframe)
{
	unsigned short rv = 0;
	if (lframe) rv = sizeof(HDRUBX) + sizeof(TUBXCRC) + lframe->Len;
	return rv;
}



// Version = 0
void TCANARDUBLOX::SendUBX_CFGVAL_set (void *InArray, unsigned long sizes, EUBXMEMTYPE lay)
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_CFG;
lFrame->Id = EUBXCFG_VALSET;
lFrame->Len = sizes;

WriteUBX_CRC (lFrame);	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}





void TCANARDUBLOX::SendUBX_SetDGNSS (ERTKSUBMODE modd)
{
UBXCFGDGNSS *lFrame = (UBXCFGDGNSS*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_DGNSS;
lFrame->Hdr.Len = sizeof(UBXCFGDGNSS) - sizeof(HDRUBX);
lFrame->dgnssMode = modd;
WriteUBX_CRC (&lFrame->Hdr);	
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));	
}



void TCANARDUBLOX::SendUBX_NAV_DOP_req ()
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_NAV;
lFrame->Id = EUBXNAV_DOP;
lFrame->Len = 0;
WriteUBX_CRC (lFrame);
	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}



void TCANARDUBLOX::SendUBX_NAV_PVT_req ()
{
HDRUBX *lFrame = (HDRUBX*)buf_tx;
WriteUBXSignature (lFrame);
lFrame->Class = EUBXCLASS_NAV;
lFrame->Id = EUBXNAV_PVT;
lFrame->Len = 0;
WriteUBX_CRC (lFrame);
	
RawTransmit (buf_tx, GetUBXFrameSize (lFrame));
}



void TCANARDUBLOX::SendUBX_CFG_RATE_req (unsigned short rate_ms)
{
UBXSETRATE *lFrame = (UBXSETRATE*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_RATE;
lFrame->Hdr.Len = sizeof(UBXSETRATE) - sizeof(HDRUBX);
lFrame->measRate = rate_ms;
lFrame->navRate = 1;	 	// 2
lFrame->timeRef = 0;		// utc
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}


void TCANARDUBLOX::SendUBX_SetSpeed (unsigned long spdset)
{
UBXSETSPEED *lFrame = (UBXSETSPEED*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_PRT;
lFrame->Hdr.Len = sizeof(UBXSETSPEED) - sizeof(HDRUBX);
lFrame->baudRate = spdset;
lFrame->portID = 1;					// usart 1
lFrame->mode = 0x8C0;				// 8 bit, no parity, 1 stop
#ifdef ZEDRTK_MODULE
	lFrame->inProtoMask = 3 | 32;			// nmea + ubx + rtcm3
	lFrame->outProtoMask = 3 | 32;		// nmea + ubx	+ rtcm3
#else
	lFrame->inProtoMask = 3;		// nmea + ubx
	lFrame->outProtoMask = 3;		// nmea + ubx	
#endif
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}



void TCANARDUBLOX::SendUBX_SetPMS ()
{
UBXSETPMS *lFrame = (UBXSETPMS*)buf_tx;
WriteUBXSignature (&lFrame->Hdr);
lFrame->Hdr.Class = EUBXCLASS_CFG;
lFrame->Hdr.Id = EUBXCFG_PMS;
lFrame->Hdr.Len = sizeof(UBXSETPMS) - sizeof(HDRUBX);
lFrame->version = 0;
lFrame->powerSetupValue = 4;			// 5 = 4 ��
lFrame->period = 0;
lFrame->onTime = 0;
WriteUBX_CRC (&lFrame->Hdr);
RawTransmit (buf_tx, GetUBXFrameSize (&lFrame->Hdr));
}



void IGPS::NVIC_USART_ENABLE (bool stat)
{
	if (stat)
		{
		NVIC_EnableIRQ (USART1_IRQn);
		}
	else
		{
		NVIC_DisableIRQ (USART1_IRQn);
		}
}



void IGPS::DeInit ()
{
NVIC_USART_ENABLE (false);
}


/*
void IGPS::Init (uint32_t speed_set)	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//USARTPort = C_USART_GPS_PORT;
	//lFS_PUSART[EUSARTINDX_GPS] = (TUSART_IFACE*)this;
	
	if (!speed_set) 
		{
		if (usart_speed_ix >= EUSARTSPD_MAX) usart_speed_ix = EUSARTSPD_9600;
		speed_set = C_USARTSPEED_INDX[usart_speed_ix];
		}
	
	NVIC_USART_ENABLE (false);	
		
	__HAL_RCC_USART1_CLK_ENABLE ();	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	//tx_size = 0;
	//USARTPort->CR1 &= ~(0xA0);		// USART_IT_RXNE & USART_IT_TXE

	//f_tx_status = false;
	
	gps_raw_reset_pin (true);				// �������� ��������� RESET_N ��� GPS ������ = 1
	GPIO_InitStructure.Pin = C_RESETGPS_N_PIN ;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;// GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	//GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	gps_raw_reset_pin (true);				// �������� ��������� RESET_N ��� GPS ������ = 1
	
	
	// RX,TX pins
	GPIO_InitStructure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;//GPIO_PULLDOWN;//GPIO_NOPULL;//GPIO_PULLUP;//GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;//GPIO_SPEED_FREQ_VERY_HIGH;//GPIO_SPEED_FREQ_LOW;//GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	
  huart1.Instance = USART1;
  huart1.Init.BaudRate = speed_set;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	
	HAL_UART_DeInit (&huart1);
	HAL_UART_Init (&huart1);
  //huart1.Init.OverSampling = UART_OVERSAMPLING_8;//UART_OVERSAMPLING_16;
  //huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  //huart1.AdvancedInit.AdvFeatureInit =UART_ADVFEATURE_NO_INIT;// UART_ADVFEATURE_MSBFIRST_INIT;//UART_ADVFEATURE_NO_INIT;

	
	push_ix = 0;
	USART_ITConfig_RX (true);	
  USART_Cmd (USARTPort, ENABLE);
	NVIC_USART_ENABLE (true);	
	
}
*/



void TCANARDUBLOX::gps_hardware_reset ()
{
	//SWVal = EGPSSW_RESET;		- ��������
}



void IGPS::gps_power_pin (bool val)
{
	if (pin_power) _pin_set_to (pin_power, val);
}





// ������������� ��������� �������� � ���������� ��� ublox - ������� �������� �� ������������ �������� ������ 460800
void IGPS::NextSpeedSearch ()
{
char cspd = usart_speed_ix; cspd++;
loc_cnt_messages = 0;
if (cspd >= EUSARTSPD_MAX) cspd = EUSARTSPD_9600; 
usart_speed_ix = (EUSARTSPD)cspd;
}




void TCANARDUBLOX::send_work_speed ()
{
	// c_work_speed_ix
//usart_speed_ix = EUSARTSPD_460800;
usart_speed_ix = c_work_speed_ix;
SendUBX_SetSpeed (C_USARTSPEED_INDX[usart_speed_ix]);
}



// 
/*
void TCANARDUBLOX::ToDetectUsartSpeed ()
{
UBXReqTimet_RTK = 0;			// ��������� ������� �������� ������� ����� rate measure
Timer_RATE = 0;
	SWVal = EGPSSW_DETECT_SPEED_SET;
	NextSpeedSearch ();
	DetectTimer = C_SPEEDSET_TIMEOUT;
}
*/



static unsigned char HexCharToBin (unsigned char datas)
{
unsigned char rv=0;
while (1)
        {
        if (datas>=97 && datas<=102)
                {
                rv=10+(datas-97);
                break;
                }
        if (datas>=65 && datas<=70)
                {
                rv=10+(datas-65);
                break;
                }
        if (datas>=48 && datas<=57)
                {
                rv=datas-48;
                break;
                }
        break;
        }
return rv;
}



static unsigned char CHex2Val (unsigned char *lpRams)
{
unsigned char rv = (HexCharToBin(lpRams[0]) << 4);
rv = rv | HexCharToBin(lpRams[1]);
return rv;
}



long IGPS::FindFistDelimitter (uint32_t start_ix, uint32_t stop_ix, uint8_t delimm)
{
long rv = -1;
uint8_t  dat;
while (start_ix < stop_ix)
    {
    dat = rxdatabuf[start_ix];
    if (!dat) break;
    if (dat == delimm)
        {
        rv = start_ix;
        break;
        }
    start_ix++;
    }
return rv;
}



bool IGPS::check_nmea_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t start_ix, uint32_t last_psh_ix)
{
	bool rv = false;
	// ������� ����������� MNEA ���������
	uint32_t ix = start_ix;
	long find_ix_s, find_ix_p;
	while (ix < last_psh_ix)
			{
			find_ix_s = FindFistDelimitter (ix, last_psh_ix, '$');

			if (find_ix_s != -1)
					{
					find_ix_p = FindFistDelimitter (find_ix_s, last_psh_ix, '*'); 
					if (find_ix_p != -1)
							{
							uint32_t full_mess_size = find_ix_p - find_ix_s + 3;  // + 1 + 2byte crc
							if (((uint32_t)find_ix_p + 2) <= last_psh_ix)  // crc in bufer ?
									{
									unsigned char CRCX = 0;
									unsigned char *lCur = &rxdatabuf[find_ix_s + 1];// lStart + 1;
									unsigned short crcsz = find_ix_p - find_ix_s - 1;// full_mess_size - 4;
									// ������� ����������� ����� NMEA
									while (crcsz)
											{
											CRCX = CRCX ^ lCur[0];
											lCur++;
											crcsz--;
											}
									unsigned char dat = CHex2Val (&rxdatabuf[find_ix_p + 1]);
									if (dat == CRCX)
											{
											mess_size = full_mess_size;
											ix_start = find_ix_s;
											rv = true;
											break;
											}
										else
											{
											// ������������ ����������� �����
											ix = find_ix_p + 1;
											}
									}
								else
									{
									// ����������� ����� �� ����� � ������
									break;
									}
							
							}
						else
							{
							// ��� ���� ���������� ������
							break;
							}
					}
				else
					{
					// � ����� ��� ��������� ������ nmea ���������
					break;
					}
			}
return rv;
}



bool IGPS::check_um_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t start_ix, uint32_t last_psh_ix)
{
bool rv = false;
uint32_t ix = start_ix, datasize;
S_UMHEADER_T *lUm;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lUm = (S_UMHEADER_T*)&rxdatabuf[ix];
    if (datasize > (sizeof(S_UMHEADER_T) + sizeof(S_UM_CRC32_T)))
        {
        if (lUm->sync_a == C_UMSYNC_A && lUm->sync_b == C_UMSYNC_B && lUm->sync_c == C_UMSYNC_C)
            {
            unsigned short full_len = sizeof(S_UMHEADER_T) + lUm->mess_length + sizeof(S_UM_CRC32_T);   // + 4 byte crc
            if (datasize >= full_len)
                {
                // �������� ����������� �����
                S_UM_CRC32_T ubx_Crc;
                CalculateUM_CRC ((uint8_t*)lUm, full_len - sizeof(S_UM_CRC32_T), &ubx_Crc);  // -4 bytes crc
								
                S_UM_CRC32_T *lfnd_Crc = (S_UM_CRC32_T*)(((uint8_t*)lUm) + sizeof(S_UMHEADER_T) + lUm->mess_length);
                if (lfnd_Crc->crc == ubx_Crc.crc)
                    {
                    ix_start = ix;
                    mess_size = full_len;
                    rv = true;
                    break;
                    }
                }
							else
								{
								// ����� ���� ������������ ���� �������
								//break;
								}
            }
        }
			else
				{
				
				break;
				}
    ix++;
    }
return rv;
}



bool IGPS::check_ubx_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t start_ix, uint32_t last_psh_ix)
{
bool rv = false;
uint32_t ix = start_ix, datasize;
HDRUBX *lUbx;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lUbx = (HDRUBX*)&rxdatabuf[ix];
    if (datasize > (sizeof(HDRUBX) + sizeof(TUBXCRC)))
        {
        if (lUbx->Preamble_A == CUBXPREX_A && lUbx->Preamble_B == CUBXPREX_B)
            {
            unsigned short full_len = sizeof(HDRUBX) + lUbx->Len + sizeof(TUBXCRC);   // + 2 byte crc
            if (datasize >= full_len)
                {
                // �������� ����������� �����
                TUBXCRC ubx_Crc;
                CalculateUBX_CRC ((uint8_t*)&lUbx->Class, full_len - 4, &ubx_Crc);  // - 2 bytes preamble, -2 bytes crc
                TUBXCRC *lfnd_Crc = (TUBXCRC*)(((uint8_t*)lUbx) + sizeof(HDRUBX) + lUbx->Len);
                if (lfnd_Crc->CK_A == ubx_Crc.CK_A && lfnd_Crc->CK_B == ubx_Crc.CK_B)
                    {
                    ix_start = ix;
                    mess_size = full_len;
                    rv = true;
                    break;
                    }
                }
            }
        }
			else
				{
				break;
				}
    ix++;
    }
return rv;
}



bool IGPS::check_rtcm_linedata (uint32_t &ix_start, uint32_t &mess_size, uint32_t start_ix, uint32_t last_psh_ix)
{
bool rv = false;
HDRTRANPLAYRTCM3 *lframe;
uint32_t ix = start_ix, datasize;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lframe = (HDRTRANPLAYRTCM3*)&rxdatabuf[ix];
    // ��������������� �������� ������� ������ ������ � ������
    if (datasize > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // 3 byte header + 3 byte qcrc
        {
        if (lframe->Preamble_D3 == 0xD3)
            {
            unsigned short len_raw = SwapShort (lframe->len_raw);
            if (len_raw && len_raw < 1023)	// 6 reserved + 10 bits size
                {
                unsigned short len = sizeof(HDRTRANPLAYRTCM3) + len_raw;
                unsigned short full_len = len + C_RTCM3_CRC24_BSIZE;      // + 3 byte qcrc
                // �������� �������� ������� ������ ������ � ������
                if (datasize >= full_len)
                    {
                    unsigned long cur_crc = crc24q_hash ((unsigned char*)lframe, len);
                    if (compareLong24AndBuf (cur_crc, ((unsigned char*)lframe) + len))
                        {
                        mess_size = full_len; 
                        ix_start = ix;
                        rv = true;
                        break;
                        }
                    }
                else
                    {
                    // size is small
                    //break;
                    }
                }
            }
        }
    else
        {
        // size is small
        break;
        }
    ix++;
    }
return rv;
}




bool TCANARDUBLOX::cb_ubx_data (HDRUBX *lUbx, uint32_t size)
{
bool rv = false;
if (lUbx)
	{
	char *lpMessUBX = ((char*)lUbx) + sizeof (HDRUBX);				
	switch (lUbx->Class)
		{
		case EUBXCLASS_NAV:
			{
			switch (lUbx->Id)
				{
				case EUBXNAV_DOP:
					{
					ParseUBXLine_DOP (lpMessUBX, lUbx->Len);
					rv = true;
					break;
					}
				case EUBXNAV_PVT:
					{
					ParseUBXLine_PVT (lpMessUBX, lUbx->Len);
					rv = true;
					break;
					}
				default:
					{
					break;
					}
				}
			break;
			}
		case EUBXCLASS_MON:
			{
			break;
			}		
		case EUBXCLASS_ACK:
			{
			switch (lUbx->Id)
				{
				case EUBXACK_NACK:
					{
					ParseUBXLine_NACK (lpMessUBX, lUbx->Len);
					rv = true;
					break;
					}
				case EUBXACK_ACK:
					{
					ParseUBXLine_ACK (lpMessUBX, lUbx->Len);
					rv = true;
					break;
					}
				}
			break;
			}
		default:
			{
			break;
			}
		}
	}
return rv;
}



bool IGPS::cb_nmea_data (uint8_t *inp, uint32_t size)
{
	bool rv = false;
	unsigned long FieldLen;			// ������ ���� � ������
	unsigned long TagMaxCount;	
	unsigned long size_line = 0;
	char *lFieldStr;
	if (inp)
		{
		GetTagStringDelimIndx ((char*)inp, 0xFF, ',', &lFieldStr, &TagMaxCount);		// ���������� ���������� ����� � nmea ������ 
		FieldLen = GetTagStringDelimIndx ((char*)inp, 0, ',', &lFieldStr, 0);				// ��������� ��������� �� �������� ���� � ������� ������� ����
		if (FieldLen >= 4)	// ����������� ���� ���� ������ (�� ������� ���� ������ ��� ����� 4)
			{
			do 	{
					
					if (str_compare((char*)ltxt_gngns, lFieldStr, FieldLen))
						{
						ParseGNGNSLine ((char*)inp, size_line, TagMaxCount);
						RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
					
					
					if (str_compare((char*)ltxt_gpgns, lFieldStr, FieldLen))
						{
						//ParseGPGNSLine ((char*)inp, size_line, TagMaxCount);
						ParseGNGNSLine ((char*)inp, size_line, TagMaxCount);
						RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
					
					if (str_compare((char*)ltxt_gngga, lFieldStr, FieldLen))
						{
						ParseGNGGALine ((char*)inp, size_line, TagMaxCount);	
						GGA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
					if (str_compare((char*)ltxt_gpgga, lFieldStr, FieldLen))
						{
						ParseGPGGALine ((char*)inp, size_line, TagMaxCount);	
						GGA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
						
					if (str_compare((char*)ltxt_gnrmc, lFieldStr, FieldLen))
						{
						ParseGNRMCLine ((char*)inp, size_line, TagMaxCount); 
						RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}		
					if (str_compare((char*)ltxt_gprmc, lFieldStr, FieldLen))
						{
						ParseGPRMCLine ((char*)inp, size_line, TagMaxCount); 
						RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}	
					
					if (str_compare((char*)ltxt_gngsa, lFieldStr, FieldLen))
						{
						ParseGNGSALine ((char*)inp, size_line, TagMaxCount); 
						GSA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
					if (str_compare((char*)ltxt_gpgsa, lFieldStr, FieldLen))
						{
						ParseGPGSALine ((char*)inp, size_line, TagMaxCount); 
						GSA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
						rv = true;
						break;
						}
					
					} while (false);
			}
		}
return rv;
}



bool IGPS::cb_rtcm3_data (uint8_t *inp, uint32_t size)
{
	bool rv = false;
	return rv;
}



void IGPS::clear_dtct_tags (S_DTPROTOCOL_T *d, uint32_t s)
{
	uint32_t ix = 0;
	while (ix < s)
		{
		d[ix].type = EDTPROTOCOL_NONE;
		ix++;
		}
}



void IGPS::rx_tasks ()
{
    uint32_t c_size;
	
    if (push_ix >= c_rx_buf_alloc_size) 
			{
			push_ix  = 0;
			return;
			}
	
		c_size = push_ix;
			
    if (c_size)
        {
				clear_dtct_tags (dtct_tags, EDTPROTOCOL_ENDENUM);
				if (detect_protocols (0, c_size))
					{
					// find fist low adress
					uint32_t ix = EDTPROTOCOL_NONE + 1;
					S_DTPROTOCOL_T c_dtct;
					c_dtct.type = EDTPROTOCOL_NONE;
					c_dtct.fr_size = 0;
					c_dtct.ix_start = c_rx_buf_alloc_size - 1;
					while (ix < EDTPROTOCOL_ENDENUM)
						{
						if (dtct_tags[ix].type != EDTPROTOCOL_NONE)
							{
							if (c_dtct.fr_size)
								{
								if (c_dtct.ix_start > dtct_tags[ix].ix_start) c_dtct = dtct_tags[ix];
								}
							else
								{
								c_dtct = dtct_tags[ix];
								}
							}
						ix++;
						}
					if (c_dtct.type != EDTPROTOCOL_NONE)
						{
						parse_protocols (c_dtct);
						loc_cnt_messages++;
						// ��������� ����� �� ����������� ���������� ������ � ������
						uint32_t last_ix = c_dtct.ix_start + c_dtct.fr_size;
						__disable_irq ();				// critical section
						uint32_t loc_push_ix = push_ix;		// update new lp 
						if (last_ix <= loc_push_ix)
								{
								uint32_t tailsize = loc_push_ix - last_ix;
								CopyMemorySDC ((char*)&rxdatabuf[last_ix], (char*)rxdatabuf, tailsize);
								push_ix = tailsize;
								}
						else
							{
							// �������������� ������
							push_ix = 0;
							}
						__enable_irq ();
						}
					}		
        }
}




bool TCANARDUBLOX::detect_protocols (uint32_t start_ix, uint32_t stop_ix)
{
	bool rv = false;
  uint32_t find_ix, find_size;
	
	if (stop_ix > start_ix)
		{
		if (check_rtcm_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_RTCM].type = EDTPROTOCOL_RTCM;
			dtct_tags[EDTPROTOCOL_RTCM].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_RTCM].ix_start = find_ix;
			rv = true;
			}
			
		if (check_nmea_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_NMEA].type = EDTPROTOCOL_NMEA;
			dtct_tags[EDTPROTOCOL_NMEA].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_NMEA].ix_start = find_ix;
			rv = true;
			}
			
		if (check_ubx_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_UBX].type = EDTPROTOCOL_UBX;
			dtct_tags[EDTPROTOCOL_UBX].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_UBX].ix_start = find_ix;
			rv = true;
			}
		}
return rv;
}



bool TCANARDUBLOX::parse_protocols (const S_DTPROTOCOL_T &d)
{
	bool rv = false;
	uint32_t find_size = d.fr_size;
	//uint32_t find_ix = d.ix_start;
	uint8_t *adr = (uint8_t*)&rxdatabuf[d.ix_start];
	switch (d.type)
		{
		case EDTPROTOCOL_NMEA:
			{
			if (find_size > 4)
				{
				uint32_t l_size = find_size - 4;
				adr += 1;       // + 1byte '$'
				rv = cb_nmea_data (adr, l_size);
				if (rv) f_new_gps_data = true;
				}
			break;
			}
		case EDTPROTOCOL_RTCM:
			{
			rv = cb_rtcm3_data (adr, find_size);
			break;
			}
		case EDTPROTOCOL_UBX:
			{
			rv = cb_ubx_data ((HDRUBX*)adr, find_size);
			break;
			}
		default: break;
		}
	return rv;
}




void IGPS::update_rx_bufer ()
{
	if (usartobj)
		{
		uint16_t sz = usartobj->Rx_check();
		if (sz)
			{
			uint8_t data;
			// isr disable
			while (sz)
				{
				if (push_ix >= c_rx_buf_alloc_size) break;
				if (!usartobj->pop (data)) break;
				rxdatabuf[push_ix++] = data;
				sz--;
				}
			// isr enabled
			}
		}
}




void TCANARDUBLOX::Task ()
{
	update_rx_bufer ();

	switch (SWVal)
		{
		case EGPSSW_SPEED_SET:
			{
			if (OtherTimer) break;
			if (usart_speed_cnt)
				{
				if (tx_processed()) break;
				gps_uart_speed_set ((EUSARTSPD)C_USARTSPEED_INDX[usart_speed_ix]);
				SendUBX_SetSpeed (C_USARTSPEED_INDX[c_work_speed_ix]);
				OtherTimer = 100;	
				SWVal = EGPSSW_RATE_SET;
				//rate_set_ix = ERATECFGTAG_GGA;
				usart_speed_cnt--;
				}
			else
				{
				gps_uart_speed_set ((EUSARTSPD)C_USARTSPEED_INDX[c_work_speed_ix]);
				SWVal = EGPSSW_WORK;
				RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
				GGA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
				GSA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
				RXTraficTimeout = C_GPS_NODATA_TIMEOUT + 100;
				OtherTimer = C_MATTX_TIMEOUT;
				}
			break;
			}
		case EGPSSW_RATE_SET:
			{
			if (!OtherTimer)
				{
				if (tx_processed()) break;
				SendUBX_CFG_RATE_req (1000);
				SWVal = EGPSSW_SPEED_SET;
				NextSpeedSearch ();
				}
			break;
			}
		case EGPSSW_WORK:
			{
			rx_tasks ();		// ������� ���� ���������� � ���������
			if (loc_cnt_messages)	
				{
				loc_cnt_messages = 0;
				RXTraficTimeout = C_GPS_NODATA_TIMEOUT;		// ������� ������
				}
				// ���� ��� ��������� � ������ ��������, ������ ����� ���������� �������� �� ���� ��������
			
			if (!OtherTimer &&  !tx_processed())
				{
				switch (swUBXreq)
					{
					case ENEXTREQ_DOP:
						{
						if (!Timer_req_DOP)
							{
							SendUBX_NAV_DOP_req ();	
							OtherTimer = C_MATTX_TIMEOUT;
							Timer_req_DOP = C_PERIOD_DOP;
							}
						swUBXreq = ENEXTREQ_PVT;
						break;
						}
					case ENEXTREQ_PVT:
						{
						if (!UBXReqTimet_PVT)
							{
							SendUBX_NAV_PVT_req ();		// ������ �� ������� ubx_pvt
							OtherTimer = C_MATTX_TIMEOUT;
							UBXReqTimet_PVT = C_PERIOD_PVT;
							}
						swUBXreq = ENEXTREQ_RATE;
						break;
						}
					case ENEXTREQ_RATE:
						{
						if (!Timer_RATE)
							{
							SendUBX_CFG_RATE_req (1000);
							OtherTimer = C_MATTX_TIMEOUT;
							Timer_RATE = C_PERIOD_RATE * 5;
							}
						swUBXreq = ENEXTREQ_DOP;
						break;
						}
					default:
						{
						swUBXreq = ENEXTREQ_DOP; 
						break;
						}
					}
				}
			
			if (!RXTraficTimeout) 
				{
				to_detect_speed ();
				}
			if (!RMC_contrl_Timer || !GGA_contrl_Timer || !GSA_contrl_Timer) 
				{
				to_detect_speed ();
				}
			break;
			}
		case EGPSSW_RESET:
			{
			gps_power_pin (false);			// �������� ��������� ����� reset
			OtherTimer = 150; // 120
			SWVal = EGPSSW_RESET_WAIT;
			// ��� break
			}						
		case EGPSSW_RESET_WAIT:
			{
			if (!OtherTimer)
				{
				gps_power_pin (true);			// ���������� ��������� reset
				to_detect_speed ();
				OtherTimer = 150;
				}
			break;
			}
		default:
			{
			break;
			}
		}
}



void IGPS::start_to_power_wait ()
{
	OtherTimer = 1100; // 120
	SWVal = EGPSSW_RESET_WAIT;
}




bool IGPS::compareLong24AndBuf (unsigned long curdata, unsigned char *lTxt)
{
bool rv = false;
if ((curdata & 0xFF) == lTxt[2] && ((curdata >> 8) & 0xFF) == lTxt[1] && ((curdata >> 16) & 0xFF) == lTxt[0]) rv = true;
return rv;	
}



bool IGPS::CheckRTCM_Format (HDRTRANPLAYRTCM3 *lframe, uint32_t *cut_sz)
{
bool rv = false;
if (lframe)
	{
	if (lframe->Preamble_D3 == 0xD3)		// magic preamble
		{
		unsigned short len_raw = SwapShort (lframe->len_raw);
		if (len_raw && len_raw < 1023)	// 6 reserved + 10 bits size
			{
			unsigned short len = sizeof(HDRTRANPLAYRTCM3) + len_raw;
			char *lEnd = (char*)rxdatabuf + c_rx_buf_alloc_size - 1;
			if (lEnd > (((char*)lframe) + len + 3))		// + 24bit qcrc
				{
				unsigned long cur_crc = crc24q_hash ((unsigned char*)lframe, len);
				if (compareLong24AndBuf (cur_crc, ((unsigned char*)lframe) + len))
					{
					rv = true;
					if (cut_sz) *cut_sz = len + 3;
					}
				}
			}
		}
	}
return rv;
}
	


bool TCANARDUBLOX::CheckUBX_Format (HDRUBX *lframe, unsigned long *cut_sz)
{
	bool rv = false;
	if (lframe)
		{
		char *lEnd = (char*)rxdatabuf + push_ix - 1;
		char *lEndFrame = (char*)lframe;
		if (lEnd > (lEndFrame + sizeof(HDRUBX)))
			{
			if (lframe->Preamble_A == CUBXPREX_A && lframe->Preamble_B == CUBXPREX_B)
				{	
				TUBXCRC cur_Crc;
				if (CalculateUBX_CRC ((unsigned char *)&lframe->Class, lframe->Len + sizeof(HDRUBX) - 2, &cur_Crc))
					{
					TUBXCRC *lfrm_crc = (TUBXCRC*)(((char*)lframe) + sizeof(HDRUBX) + lframe->Len);
					if (lfrm_crc->CK_A == cur_Crc.CK_A && lfrm_crc->CK_B == cur_Crc.CK_B) 
						{
						if (cut_sz) *cut_sz = sizeof(HDRUBX) + lframe->Len + sizeof(TUBXCRC);
						rv = true;
						}
					}
				}
			}
		}
	return rv;
}




bool IGPS::ParseGNGNSLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_GNSS_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;

		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_UTC, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldUTC (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LAT, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldLatitude (lFieldStr, LenField);
			
		// lat pole parse 
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LAT_NS, ',', &lFieldStr, 0);
		nmea_ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse 
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LONG, ',', &lFieldStr, 0);
		nmea_ParseFieldLongitude (lFieldStr, LenField);
			
		// long polus parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_LON_EW, ',', &lFieldStr, 0);
		nmea_ParseFieldLongEW (lFieldStr, LenField);
		
		/* ������ � ������ ����� �� PVT
		// quality parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_QUAL, ',', &lFieldStr, 0);
		ParseFieldPosMode (lFieldStr, LenField);	
		*/
		
		// satelite numbers parse
		
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_NBRSSATEL, ',', &lFieldStr, 0);
		nmea_ParseFieldNmbsSatelite (lFieldStr, LenField);
		
		
		
		// Altitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_ALTIT, ',', &lFieldStr, 0);
		nmea_ParseFieldAltitude (lFieldStr, LenField);
		
		// dif elipsoide parse
		LenField = GetTagStringDelimIndx (lTxt, EGNSSINX_D_ELIPS, ',', &lFieldStr, 0);
		nmea_ParseFieldDifElipsHeight (lFieldStr, LenField);
		
		}
	return rv;
}



// UM980
bool IGPS::ParseGNGGALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_GGA_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;

		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_UTC, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldUTC (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldLatitude (lFieldStr, LenField);
			
		// lat pole parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT_NS, ',', &lFieldStr, 0);
		nmea_ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LONG, ',', &lFieldStr, 0);
		nmea_ParseFieldLongitude (lFieldStr, LenField);
			
		// long polus parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LON_EW, ',', &lFieldStr, 0);
		nmea_ParseFieldLongEW (lFieldStr, LenField);
		
		// quality parse in other message
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_QUAL, ',', &lFieldStr, 0);
		nmea_ParseFieldGGAQuality (lFieldStr, LenField);
		update_fix2_gpsmode_status ();
		
		// satelite numbers parse
		
		//LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_NBRSSATEL, ',', &lFieldStr, 0);
		//nmea_ParseFieldNmbsSatelite (lFieldStr, LenField);
		
		
		// hdop parse in other message
		
		// Altitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_ALTIT, ',', &lFieldStr, 0);
		nmea_ParseFieldAltitude (lFieldStr, LenField);
		
		// dif elipsoide parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_D_ELIPS, ',', &lFieldStr, 0);
		nmea_ParseFieldDifElipsHeight (lFieldStr, LenField);
		}
	return rv;
}



// UBLOX
bool IGPS::ParseGPGGALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_GGA_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;

		// utc parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_UTC, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldUTC (lFieldStr, LenField);
			
		// latitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldLatitude (lFieldStr, LenField);
			
		// lat pole parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LAT_NS, ',', &lFieldStr, 0);
		nmea_ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse 
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LONG, ',', &lFieldStr, 0);
		nmea_ParseFieldLongitude (lFieldStr, LenField);
			
		// long polus parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_LON_EW, ',', &lFieldStr, 0);
		nmea_ParseFieldLongEW (lFieldStr, LenField);
		
		// quality parse in other message
		
		// satelite numbers parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_NBRSSATEL, ',', &lFieldStr, 0);
		nmea_ParseFieldNmbsSatelite (lFieldStr, LenField);
		
		// hdop parse in other message
		
		// Altitude parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_ALTIT, ',', &lFieldStr, 0);
		nmea_ParseFieldAltitude (lFieldStr, LenField);
		
		// dif elipsoide parse
		LenField = GetTagStringDelimIndx (lTxt, EGGAINDX_D_ELIPS, ',', &lFieldStr, 0);
		nmea_ParseFieldDifElipsHeight (lFieldStr, LenField);
		}
	return rv;
}



bool IGPS::ParseGNRMCLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_RMC_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MODEIND, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldPosMode (lFieldStr, LenField);
		// bool IGPS::nmea_ParseFieldPosMode (char *lTxt, unsigned long sizes)
			
		// utc parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTC, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldUTC (lFieldStr, LenField);
			
		// valid parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_VALID, ',', &lFieldStr, 0);
		//nmea_ParseFieldValid (lFieldStr, LenField);
			
		// latitude parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldLatitude (lFieldStr, LenField);
			
		// latit poluse parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT_NS, ',', &lFieldStr, 0);
		//nmea_ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LONG, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldLongitude (lFieldStr, LenField);
		
		// longitude polus parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LON_EW, ',', &lFieldStr, 0);
		//nmea_ParseFieldLongEW (lFieldStr, LenField);
		
		// speedov parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_SPEEDOV, ',', &lFieldStr, 0);
		nmea_ParseFieldSpeedOV (lFieldStr, LenField);
		
		// course parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_COURSE, ',', &lFieldStr, 0);
		nmea_ParseFieldCourse (lFieldStr, LenField);
		
		// UTD parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTD, ',', &lFieldStr, 0);
		nmea_ParseFieldUTD (lFieldStr, LenField);
		
		// magnet var parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGNETVAR, ',', &lFieldStr, 0);
		nmea_ParseFieldMagnetVar (lFieldStr, LenField);
		
		// EW parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGEW, ',', &lFieldStr, 0);
		nmea_ParseFieldMagnetEW (lFieldStr, LenField);
		}
	return rv;
}



bool IGPS::ParseGPRMCLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
	bool rv = false;
	if (amount_filed >= C_RMC_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// utc parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTC, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldUTC (lFieldStr, LenField);
			
		// valid parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_VALID, ',', &lFieldStr, 0);
		//nmea_ParseFieldValid (lFieldStr, LenField);
			
		// latitude parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldLatitude (lFieldStr, LenField);
			
		// latit poluse parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LAT_NS, ',', &lFieldStr, 0);
		//nmea_ParseFieldLatNS (lFieldStr, LenField);
			
		// longitude parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LONG, ',', &lFieldStr, 0);
		//rv |= nmea_ParseFieldLongitude (lFieldStr, LenField);
		
		// longitude polus parse
		//LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_LON_EW, ',', &lFieldStr, 0);
		//nmea_ParseFieldLongEW (lFieldStr, LenField);
		
		// speedov parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_SPEEDOV, ',', &lFieldStr, 0);
		nmea_ParseFieldSpeedOV (lFieldStr, LenField);
		
		// course parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_COURSE, ',', &lFieldStr, 0);
		nmea_ParseFieldCourse (lFieldStr, LenField);
		
		// UTD parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_UTD, ',', &lFieldStr, 0);
		nmea_ParseFieldUTD (lFieldStr, LenField);
		
		// magnet var parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGNETVAR, ',', &lFieldStr, 0);
		nmea_ParseFieldMagnetVar (lFieldStr, LenField);
		
		// EW parse
		LenField = GetTagStringDelimIndx (lTxt, ERMCINDX_MAGEW, ',', &lFieldStr, 0);
		nmea_ParseFieldMagnetEW (lFieldStr, LenField);
		}
	return rv;
}



bool IGPS::ParseGNGSALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
bool rv = false;
		if (amount_filed >= C_GSA_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// no fix = 0, 2d = 1, 3d = 2
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_FIXMOD, ',', &lFieldStr, 0);
		rv = nmea_ParseFieldModeNF2D3D (lFieldStr, LenField);
		update_fix2_gpsmode_status ();
			
		/*
		// pdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_PDOP, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldPDOP (lFieldStr, LenField);
			
		// hdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_HDOP, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldHDOP (lFieldStr, LenField);
			
		// vdop parse
		LenField = GetTagStringDelimIndx (lTxt, EGSAINX_VDOP, ',', &lFieldStr, 0);
		rv |= nmea_ParseFieldVDOP (lFieldStr, LenField);
		*/
		}
return rv;		
}



bool IGPS::ParseGPGSALine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
return ParseGNGSALine (lTxt, size, amount_filed);		
}



/*
bool IGPS::ParseGSVLine (char *lTxt, unsigned long size, unsigned long amount_filed)
{
bool rv = false;
		if (amount_filed >= C_GSV_MAXFIELD)
		{
		char *lFieldStr;
		unsigned long LenField;
			
		// satelite view number parse
		LenField = GetTagStringDelimIndx (lTxt, EGSVINX_VISIBSAT, ',', &lFieldStr, 0);
		rv = nmea_ParseFieldSatView (lFieldStr, LenField);
		
		}
return rv;
}
*/

/*
bool IGPS::nmea_ParseFieldSatView(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >= 2)
	{
	unsigned long DataOut = 0;
	do  {
			// satelit view count
			if (!TxtToULong (lTxt, sizes, &DataOut)) break;
			if (DataOut <= 127)
				{
				GPSAccData.satelitevieww.data = DataOut;
				GPSAccData.satelitevieww.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
				rv = true;
				}
			} while (false);
	}
return rv;
}
*/



bool IGPS::nmea_ParseFieldPDOP(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.PDOP.value = outd;
		GPSAccData.PDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;	
}



bool IGPS::nmea_ParseFieldVDOP(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >=3)
	{
	float outd = 0;
	sizes = 3;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.VDOP.value = outd;
		GPSAccData.VDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;	
}



bool IGPS::nmea_ParseFieldHDOP (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.HDOP.value = outd;
		GPSAccData.HDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;	
}



bool IGPS::nmea_ParseFieldModeNF2D3D(char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	if (sizes == 1) {
		char dat = *lTxt;
		if (dat >= '0' && dat <= '3')
			{
			dat -= '0';
			GPSAccData.mode2d3d_gsa.data = dat;
			GPSAccData.mode2d3d_gsa.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			}
		}
	}
return rv;	
}



bool IGPS::nmea_ParseFieldMagnetEW (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	if (sizes == 1) {
		char dat = lTxt[0];
		switch (dat)
			{
			case 'W':  case 'E': 
				{
				GPSAccData.mag_EW.data = dat;
				GPSAccData.mag_EW.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
				rv = true;
				break;
				}
			}
		}
	}

return rv;	
}



bool IGPS::nmea_ParseFieldMagnetVar (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.MagVar.data = outd;
		GPSAccData.MagVar.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;
}



bool IGPS::nmea_ParseFieldCourse (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes)) {
		GPSAccData.TrCourse.data = outd;
		GPSAccData.TrCourse.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;
}



bool IGPS::nmea_ParseFieldUTD (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes >= 6)
	{
	unsigned long DataOut = 0;
	do  {
			// ����
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Day = DataOut;
		
			// �����
			lTxt += 2;
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Month = DataOut;
		
			// ���
			lTxt += 2;
			if (!TxtToULong (lTxt, 2, &DataOut)) break;
			GPSAccData.UTD_date.Year = DataOut;
		
			rv = true;
			GPSAccData.UTD_date.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			} while (false);
	}
return rv;
}



bool IGPS::nmea_ParseFieldSpeedOV (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.SpeedData.data = outd;
		GPSAccData.SpeedData.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}

return rv;
}
	


/*
bool IGPS::nmea_ParseFieldValid (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes == 1)
		{
		char dat = lTxt[0];
		switch (dat)
			{
			case 'A': case 'V':
				{
				GPSAccData.validitystatus.data = dat;
				rv = true;
				break;
				}
			}
		}
	}
GPSAccData.validitystatus.hdr.valid_timeout = rv?1500:0;
return rv;
}
*/


bool IGPS::nmea_ParseFieldUTC (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	if (sizes >= 6)
		{
		unsigned long DataOut = 0;
		float outd;
		do  {
				// ����
				if (!TxtToULong (lTxt, 2, &DataOut)) break;
				GPSAccData.UTC_time.Hour = DataOut;
			
				// ������
				lTxt += 2;
				if (!TxtToULong (lTxt, 2, &DataOut)) break;
				GPSAccData.UTC_time.Minute = DataOut;
			
				// ������� ����� ���� float
				lTxt += 2;
				sizes -= 4;
				if (!TxtToFloat (&outd, lTxt, sizes)) break;
				GPSAccData.UTC_time.seconds = outd;
			
				rv = true;
				} while (false);
		}
	}
	
	if (rv)		// utc ������ ��������
		{
		// ������������ ��������� gps ������ �� ��������� utc ��������
		if (utc_last_seconds_value != GPSAccData.UTC_time.seconds)
			{
			utc_last_seconds_value = GPSAccData.UTC_time.seconds;
			GPSAccData.UTC_time.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			}
		}
return rv;	
}





bool IGPS::nmea_ParseFieldLatNS (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	if (sizes == 1) {
		char dat = lTxt[0];
		switch (dat)
			{
			case 'N':  case 'S': 
				{
				GPSAccData.pos_pole_NS.data = dat;
				GPSAccData.pos_pole_NS.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
				rv = true;
				break;
				}
			}
		}
	}
return rv;
}



bool IGPS::nmea_ParseFieldLatitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes > 2) {
	float outd = 0;
	unsigned long Angle;
	// ������� ������
	do {
			if (!TxtToULong (lTxt, C_NMEASIZELAT_DEG, &Angle)) break;		// ��� �������
			sizes -= C_NMEASIZELAT_DEG; lTxt += C_NMEASIZELAT_DEG;
			// ������ ������
			if (!TxtToFloat (&outd, lTxt, sizes)) break;
			GPSAccData.Pos_lat.value = outd; GPSAccData.Pos_lat.value = (GPSAccData.Pos_lat.value / 60) + Angle;
			GPSAccData.Pos_lat.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;	
			} while (false);
	}
return rv;
}



bool IGPS::nmea_ParseFieldLongitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes > 5)
	{
	float outd = 0;
	unsigned long Angle;
	// ������� �������
	do {
			if (!TxtToULong (lTxt, C_NMEASIZELONG_DEG, &Angle)) break;		// ��� �������
			sizes -= C_NMEASIZELONG_DEG; lTxt += C_NMEASIZELONG_DEG;
			// ������ �������
			if (!TxtToFloat (&outd, lTxt, sizes)) break;
			GPSAccData.Pos_lon.value = outd; GPSAccData.Pos_lon.value = (GPSAccData.Pos_lon.value / 60) + Angle;
			GPSAccData.Pos_lon.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;	
			} while (false);
	}
return rv;
}



bool IGPS::nmea_ParseFieldLongEW (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	if (sizes == 1) {
		char dat = lTxt[0];
		switch (dat)
			{
			case 'E': case 'W': 
				{
				GPSAccData.pos_pole_EW.data = dat;
				GPSAccData.pos_pole_EW.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
				rv = true;
				break;
				}
			}
		}
	}
return rv;
}





/*
uint2 STATUS_NO_FIX    = 0
uint2 STATUS_TIME_ONLY = 1
uint2 STATUS_2D_FIX    = 2
uint2 STATUS_3D_FIX    = 3
uint2 status

#
# GNSS Mode
#
uint4 MODE_SINGLE      = 0
uint4 MODE_DGPS        = 1
uint4 MODE_RTK         = 2
uint4 MODE_PPP         = 3
uint4 mode

#
# GNSS Sub mode
#
uint6 SUB_MODE_DGPS_OTHER    = 0
uint6 SUB_MODE_DGPS_SBAS     = 1

uint6 SUB_MODE_RTK_FLOAT     = 0
uint6 SUB_MODE_RTK_FIXED     = 1


*/


bool IGPS::nmea_ParseFieldGGAQuality (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes) {
	if (sizes == 1) {
		char dat = lTxt[0] - '0';
		if (dat <= 6) {
			GPSAccData.modequality_gga.data = dat;
			GPSAccData.modequality_gga.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			}
		}
	}
return rv;
}



void IGPS::update_fix2_gpsmode_status ()
{
	if (GPSAccData.mode2d3d_gsa.data > 1 && GPSAccData.mode2d3d_gsa.data <= 3 && GPSAccData.mode2d3d_gsa.hdr.valid_timeout)
		{
		bool f_ok = false;
		if (GPSAccData.modequality_gga.hdr.valid_timeout)
			{
			switch (GPSAccData.modequality_gga.data)
				{
				case 1:		// 2D, 3D
					{
					GPSAccData.rtk_status.data = 0;				// MODE_SINGLE
					GPSAccData.dgps_submode.data = 0;
					f_ok = true;										
					break;
					}
				case 2:		// DGPS
					{
					GPSAccData.rtk_status.data = 1;				// MODE_DGPS
					GPSAccData.dgps_submode.data = 0;
					f_ok = true;
					break;
					}
				case 4:		// RTK INT
					{
					GPSAccData.rtk_status.data = 2;				// MODE_RTK
					GPSAccData.dgps_submode.data = 1;			// SUB_MODE_RTK_FIXED
					f_ok = true;
					break;
					}
				case 5:		// RTK FLOAT
					{	
					GPSAccData.rtk_status.data = 2;				// MODE_RTK
					GPSAccData.dgps_submode.data = 0;			// SUB_MODE_RTK_FLOAT
					f_ok = true;
					break;
					}
				}
			}
		if (f_ok) 
			{
			GPSAccData.modeFIX2.data = GPSAccData.mode2d3d_gsa.data;
			GPSAccData.modeFIX2.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.rtk_status.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;	
			GPSAccData.dgps_submode.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			}
		}
}





bool IGPS::nmea_ParseFieldNmbsSatelite (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes && sizes <= 3)
	{
	unsigned long DataOut = 0;
	if (TxtToULong (lTxt, sizes, &DataOut))
		{
		GPSAccData.sateliteused.data = DataOut;
		GPSAccData.sateliteused.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}

return rv;
}





bool IGPS::nmea_ParseFieldAltitude (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.Altitud.data = outd;
		GPSAccData.Altitud.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}

return rv;
}



bool IGPS::nmea_ParseFieldDifElipsHeight (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes)
	{
	float outd = 0;
	if (TxtToFloat (&outd, lTxt, sizes))
		{
		GPSAccData.Difelipsheight.data = outd;
		GPSAccData.Difelipsheight.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	}
return rv;
}


/*
bool IGPS::GetFixData (uavcan_equipment_gnss_Fix2 &datt)
{
	bool rv = check_fix_data ();
	if (!rv)
		{
		uint32_t ticks = SYSBIOS::GetTickCountLong ();
		bool sigbit = ticks & 512;
		if (!sigbit)
			{
			f_blank_gen_fix2 = false;
			}
		else
			{
			if (f_blank_gen_fix2 != sigbit)
				{
				GPSAccData.mode2d3d_gsa.hdr.valid_timeout = 0;
				f_blank_gen_fix2 = sigbit;
				rv = true;
				}
			}
		}
	if (rv)
		{
		memset (&datt, 0, sizeof(datt));
		bool f_nofix = check_no_fix_criteria ();
		if (f_nofix)
				{
				GPSAccData.Pos_lat.hdr.valid_timeout = 0;
				GPSAccData.Pos_lon.hdr.valid_timeout = 0;
				GPSAccData.UTC_time.hdr.valid_timeout = 0;
				GPSAccData.PDOP.hdr.valid_timeout = 0;
				GPSAccData.modeFIX2.hdr.valid_timeout = 0;
				GPSAccData.Difelipsheight.hdr.valid_timeout = 0;
				GPSAccData.Altitud.hdr.valid_timeout = 0;
				GPSAccData.VELN.hdr.valid_timeout = 0;
				GPSAccData.VELE.hdr.valid_timeout = 0;
				GPSAccData.VELD.hdr.valid_timeout = 0;
				}	
			
		// latitude
		if (GPSAccData.Pos_lat.hdr.valid_timeout && GPSAccData.pos_pole_NS.hdr.valid_timeout)
			{
			datt.latitude_deg_1e8 = ((double)GPSAccData.Pos_lat.value) * 1E8;
			if (GPSAccData.pos_pole_NS.data == 'S') datt.latitude_deg_1e8 *= -1;
			}
		else
			{
			// ���� ��� ����������� N S
			datt.latitude_deg_1e8 = 0;
			}
		// longitude
		if (GPSAccData.Pos_lon.hdr.valid_timeout && GPSAccData.pos_pole_EW.hdr.valid_timeout)
			{
			datt.longitude_deg_1e8 = ((double)GPSAccData.Pos_lon.value) * 1E8;		
			if (GPSAccData.pos_pole_EW.data == 'W') datt.longitude_deg_1e8 *= -1;
			}
		else
			{
			// ���� ��� ����������� E W
			datt.longitude_deg_1e8 = 0;
			}
		if (GPSAccData.sateliteused.hdr.valid_timeout) 
			{
			datt.sats_used = GPSAccData.sateliteused.data;
			}
		else
			{
			datt.sats_used = 0;	
			}
		cur_sat_amount = datt.sats_used;
		if (GPSAccData.UTC_time.hdr.valid_timeout && GPSAccData.UTC_time.hdr.valid_timeout) 
			{
			uint32_t dval = GPSAccData.UTC_time.Hour; 
			dval *= 3600;
			dval += (((uint32_t)GPSAccData.UTC_time.Minute) * 60);
			dval += GPSAccData.UTC_time.seconds;
			uint64_t dayamount = GetUTCDaysToDate (2000 + GPSAccData.UTD_date.Year, GPSAccData.UTD_date.Month, GPSAccData.UTD_date.Day);
			dayamount *= 86400;
			datt.gnss_timestamp.usec = (dayamount + dval) * 1000000;
				
			datt.gnss_time_standard = 2;		// utc
			}
		else
			{
			// ���� ������ �� �������
			datt.gnss_timestamp.usec = 0;
			datt.gnss_time_standard = 0;		// unknow
			}
		if (GPSAccData.PDOP.hdr.valid_timeout)
			{
			datt.pdop = GPSAccData.PDOP.value;
			}
		else
			{
			datt.pdop = C_DOP_NAN;	
			}
		if (GPSAccData.modeFIX2.hdr.valid_timeout && GPSAccData.sateliteused.hdr.valid_timeout)
			{
			datt.status = GPSAccData.modeFIX2.data;
			if (GPSAccData.modeFIX2.data == 3) {
				if (GPSAccData.sateliteused.data <= 3 || !GPSAccData.Altitud.hdr.valid_timeout) datt.status = 2;		// 2d
				}
			}
		else
			{
			datt.status = 0;
			}
		gpsDmode = datt.status;
			
		if (GPSAccData.Difelipsheight.hdr.valid_timeout && GPSAccData.Altitud.hdr.valid_timeout )	// ����������� ������ ������������� �� ������ ��� �����
			{
			float elips_h = (GPSAccData.Altitud.data + GPSAccData.Difelipsheight.data) * 1000;		// ����� � ���������
			datt.height_ellipsoid_mm = elips_h;
			}
		else
			{
			datt.height_ellipsoid_mm  = 0;
			}

		if (GPSAccData.Altitud.hdr.valid_timeout)
			{
			float dat = GPSAccData.Altitud.data * 1000;		// ����� � ���������
			datt.height_msl_mm = dat;
			}
		else
			{
			datt.height_msl_mm = 0;
			}

		if (GPSAccData.VELN.hdr.valid_timeout)
			{
			datt.ned_velocity[0] = GPSAccData.VELN.value_m_s;
			}
		else
			{
			datt.ned_velocity[0] = 0;
			}
		
		if (GPSAccData.VELE.hdr.valid_timeout)
			{
			datt.ned_velocity[1] = GPSAccData.VELE.value_m_s;
			}
		else
			{
			datt.ned_velocity[1] = 0;
			}

		if (GPSAccData.VELD.hdr.valid_timeout)
			{
			datt.ned_velocity[2] = GPSAccData.VELD.value_m_s;
			}
		else
			{
			datt.ned_velocity[2] = 0;
			}
			
		datt.num_leap_seconds = 27;
			
		rv = true;
		}
	return rv;
}
*/



void IGPS::clear_covariances ()
{
memset (&GPSAccData.covs, 0, sizeof(GPSAccData.covs));
GPSAccData.covs.f_blank = true;
}


/*
DATACOVS *IGPS::GetCovariances ()
{
	if (check_no_fix_criteria ()) GPSAccData.covs.hdr.valid_timeout = 0;
	if (!GPSAccData.covs.hdr.valid_timeout) 
		{
		if (!GPSAccData.covs.f_blank) clear_covariances ();
		}
	else
		{
		GPSAccData.covs.f_blank = false;
		}
	return &GPSAccData.covs;
}
*/



bool IGPS::check_no_fix_criteria ()
{
		bool f_nofix = false;
		if (!GPSAccData.mode2d3d_gsa.hdr.valid_timeout || !GPSAccData.sateliteused.hdr.valid_timeout) f_nofix = true;
		if (!f_nofix) {
			if (GPSAccData.mode2d3d_gsa.data == 0 || GPSAccData.sateliteused.data == 0) f_nofix = true;		
			}
		return f_nofix;
}


/*
bool IGPS::GetAuxData (uavcan_equipment_gnss_Auxiliary &datt)
{
	bool rv = check_aux_data ();
	if (!rv)
		{
		uint32_t ticks = SYSBIOS::GetTickCountLong ();
		bool sigbit = ticks & 512;
		if (!sigbit)
			{
			f_blank_gen_aux = false;
			}
		else
			{
			if (f_blank_gen_aux != sigbit)
				{
				GPSAccData.mode2d3d_gsa.hdr.valid_timeout = 0;
				f_blank_gen_aux = sigbit;
				rv = true;
				}
			}
		}
		
	if (rv)
		{
		memset (&datt, 0, sizeof(datt));
		bool f_nofix = check_no_fix_criteria ();
		if (f_nofix)
				{
				GPSAccData.GDOP.hdr.valid_timeout = 0;
				GPSAccData.PDOP.hdr.valid_timeout = 0;
				GPSAccData.TDOP.hdr.valid_timeout = 0;
				GPSAccData.VDOP.hdr.valid_timeout = 0;
				GPSAccData.HDOP.hdr.valid_timeout = 0;
				GPSAccData.NDOP.hdr.valid_timeout = 0;
				GPSAccData.EDOP.hdr.valid_timeout = 0;
				}	
				
			
		if (GPSAccData.sateliteused.hdr.valid_timeout) 
			{
			datt.sats_used = GPSAccData.sateliteused.data;
			}
		else
			{
			datt.sats_used = 0;
			}
		if (GPSAccData.satelitevieww.hdr.valid_timeout)
			{
			datt.sats_visible = GPSAccData.satelitevieww.data;	
			}
		else
			{
			datt.sats_visible = 0;	
			}
		// ----------- gdop
		if (GPSAccData.GDOP.hdr.valid_timeout)
			{
			datt.gdop = GPSAccData.GDOP.value;
			}
		else
			{
			datt.gdop = C_DOP_NAN;
			}
		// ----------- pdop
		if (GPSAccData.PDOP.hdr.valid_timeout)
			{
			datt.pdop = GPSAccData.PDOP.value;
			}
		else
			{
			datt.pdop = C_DOP_NAN;
			}
		// ----------- tdop
		if (GPSAccData.TDOP.hdr.valid_timeout)
			{
			datt.tdop = GPSAccData.TDOP.value;
			}
		else
			{
			datt.tdop = C_DOP_NAN;
			}
		// ----------- vdop
		if (GPSAccData.VDOP.hdr.valid_timeout)
			{
			datt.vdop = GPSAccData.VDOP.value;
			}
		else
			{
			datt.vdop = C_DOP_NAN;
			}
		// ----------- hdop
		if (GPSAccData.HDOP.hdr.valid_timeout)
			{
			datt.hdop = GPSAccData.HDOP.value;
			}
		else
			{
			datt.hdop = C_DOP_NAN;	
			}
		// ----------- ndop
		if (GPSAccData.NDOP.hdr.valid_timeout)
			{
			datt.ndop = GPSAccData.NDOP.value;
			}
		else
			{
			datt.ndop = C_DOP_NAN;
			}
		// ----------- edop
		if (GPSAccData.EDOP.hdr.valid_timeout)
			{
			datt.edop = GPSAccData.EDOP.value;
			}
		else
			{
			datt.edop = C_DOP_NAN;
			}
		rv = true;
		}
	return rv;
}
*/



bool IGPS::tx_processed ()
{
bool rv = false;
if (usartobj) rv = usartobj->Tx_status();
return rv;
}



bool IGPS::check_fix_data ()
{
return f_new_gps_data;
}



void IGPS::clr_fix_flag ()
{
 f_new_gps_data = false;
}





/*
bool TCANARDGPS::ParseUBXLine_NAVPOSLLH (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}


unsigned long debg_one[3];
unsigned long debg_zero[3];



bool TCANARDGPS::ParseUBXLine_NAVRELPOSNED (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXRELPOSNED))
		{
		FUBXRELPOSNED *lFr = (FUBXRELPOSNED*)lTxt;
		lFr->accD = lFr->accD;
		unsigned char indx = 0;
		unsigned char mask = 1;
		unsigned long *lpInc = 0;
		while (indx < 3)
			{
			if (lFr->flags & mask)
				{
				lpInc = &debg_one[indx];	
				}
			else
				{
				lpInc = &debg_zero[indx];	
				}
			lpInc[0]++;
			mask = mask << 1;
			indx++;
			}
		rv = true;
		}
return rv;	
}



bool TCANARDGPS::ParseUBXLine_NAVSTATUS (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}



bool TCANARDGPS::ParseUBXLine_NAVSVIN (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes)
		{

		rv = true;
		}
return rv;	
}

*/



// ------------------------------- UBX ������� -----------------------------------
/*
bool TCANARDGPS::ParseUBX_OneLine (char *lpLine, unsigned long &d_size)
{
bool rv = false;
if (lpLine)
	{
	HDRUBX *lUbx = (HDRUBX*)lpLine;
	if (lUbx->Preamble_A == CUBXPREX_A && lUbx->Preamble_B == CUBXPREX_B)
		{
		// �������� ����������� �����
		unsigned char *lSrc = &lUbx->Class;
		unsigned short sz = sizeof(HDRUBX) - 2 + lUbx->Len;
		unsigned long cur_sz = 0;
		TUBXCRC ubx_Crc;
		CalculateUBX_CRC (lSrc, sz, &ubx_Crc);
		TUBXCRC *lfnd_Crc = (TUBXCRC*)(lpLine + sizeof(HDRUBX) + lUbx->Len);
		char *lpMessUBX = lpLine + sizeof (HDRUBX);
		if (lfnd_Crc->CK_A == ubx_Crc.CK_A && lfnd_Crc->CK_B == ubx_Crc.CK_B)
			{
			// ������������ ����� �������
			// ��������� ������� ���������
			cur_sz = lUbx->Len + sizeof(HDRUBX) + sizeof(TUBXCRC);
				
			switch (lUbx->Class)
				{
				case EUBXCLASS_NAV:
					{
					switch (lUbx->Id)
						{
						// ������ ��� qground
						case EUBXNAV_VELNED:
						case EUBXNAV_SAT:
							{
							rv = true;
							break;
							}
						case EUBXNAV_DOP:					// ��� ������ � �������
							{
							rv = ParseUBXLine_DOP (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXNAV_PVT:					// ��� ������ � �������
							{
							rv = ParseUBXLine_PVT (lpMessUBX, lUbx->Len);
							break;
							}
						default:
							{
							rv = false;
							break;
							}
						}
					break;
					}
				case EUBXCLASS_MON:
					{
					// ������ ��� qground
					if (lUbx->Id == EUBXMON_RF) rv = true;
					break;
					}		
				case EUBXCLASS_ACK:
					{
					switch (lUbx->Id)
						{
						case EUBXACK_NACK:
							{
							rv = ParseUBXLine_NACK (lpMessUBX, lUbx->Len);
							break;
							}
						case EUBXACK_ACK:
							{
							rv = ParseUBXLine_ACK (lpMessUBX, lUbx->Len);
							break;
							}
						}
					break;
					}
				default:
					{
					rv = rv;
					break;
					}
				}
				
			}
		if (rv) d_size = cur_sz;
		}
	}
return rv;
}
*/



bool TCANARDUBLOX::CheckUBXUpdateDOP (unsigned short inp_val, TTDOPTAG &dopval)
{
	bool rv = false;
	if (inp_val && inp_val < 10000)
		{
		float dat = (float)inp_val;
		dopval.value = dat * (float)0.01;
		dopval.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	else
		{
		dopval.hdr.valid_timeout = 0;
		dopval.value = C_DOP_NANVALUE;
		}
	return rv;
}



bool TCANARDUBLOX::ParseUBXLine_DOP (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXDOP))
		{
		FUBXDOP *ldop = (FUBXDOP*)lTxt;
		// PDOP parse
		rv = CheckUBXUpdateDOP (ldop->pDOP, GPSAccData.PDOP);
		// VDOP parse
		rv |= CheckUBXUpdateDOP (ldop->vDOP, GPSAccData.VDOP);
		// HDOP parse
		rv |= CheckUBXUpdateDOP (ldop->hDOP, GPSAccData.HDOP);	
		// EDOP parse
		rv |= CheckUBXUpdateDOP (ldop->eDOP, GPSAccData.EDOP);
		// TDOP parse
		rv |= CheckUBXUpdateDOP (ldop->tDOP, GPSAccData.TDOP);
		// GDOP parse
		rv |= CheckUBXUpdateDOP (ldop->gDOP, GPSAccData.GDOP);	
		// NDOP parse
		rv |= CheckUBXUpdateDOP (ldop->nDOP, GPSAccData.NDOP);
		//f_new_aux_data = rv;
		}
return rv;	
}



bool TCANARDUBLOX::CheckUBXUpdateVelocity (long inp_val, TTVELOCITYTAG &dopval)
{
	bool rv = false;
	if (inp_val)
		{
		float dat = inp_val;
		dopval.value_m_s = dat * 0.001F;
		dopval.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
		rv = true;
		}
	else
		{
		dopval.hdr.valid_timeout = 0;
		dopval.value_m_s = 0;
		}
	return rv;
}




bool TCANARDUBLOX::ParseUBXLine_NACK (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes == sizeof(FUBXNACK))
	{
	FUBXNACK *lnack = (FUBXNACK*)lTxt;
	switch (lnack->clsID)
		{
		default:
			{
			rv = true;
			break;
			}
		}
	rv = true;
	}
return rv;
}



bool TCANARDUBLOX::ParseUBXLine_ACK (char *lTxt, unsigned long sizes)
{
bool rv = false;
if (lTxt && sizes == sizeof(FUBXNACK))
	{
	FUBXNACK *lnack = (FUBXNACK*)lTxt;
	switch (lnack->clsID)
		{
		default:
			{
			rv = true;
			break;
			}
		}
	rv = true;
	}
return rv;
}



bool TCANARDUBLOX::ParseUBXLine_PVT (char *lTxt, unsigned long sizes)
{
bool rv = false;
	if (lTxt && sizes == sizeof(FUBXPVT))
		{
		FUBXPVT *lpvt = (FUBXPVT*)lTxt;
		float dat;
		CheckUBXUpdateVelocity (lpvt->velN, GPSAccData.VELN);
		CheckUBXUpdateVelocity (lpvt->velE, GPSAccData.VELE);
		CheckUBXUpdateVelocity (lpvt->velD, GPSAccData.VELD);
			
		if (lpvt->flags & 2)
			{
				Timer_RTK_implement = C_TIME_LED_RTK_STATUS;
				//sst_rtk = true;
			}
			
		if (GPSAccData.VELN.hdr.valid_timeout)
			{
			dat = lpvt->hAcc;
			dat *= (float)0.001;
			GPSAccData.covs.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.covs.pos_y_cov = dat;
			}
		else
			{
			GPSAccData.covs.hdr.valid_timeout = 0;
			GPSAccData.covs.pos_y_cov = 0;
			}
			
		if (GPSAccData.VELE.hdr.valid_timeout)
			{
			dat = lpvt->hAcc;
			dat *= (float)0.001;
			GPSAccData.covs.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.covs.pos_x_cov = dat;
			}
		else
			{
			GPSAccData.covs.hdr.valid_timeout = 0;
			GPSAccData.covs.pos_x_cov = 0;
			}
				
		if (GPSAccData.VELD.hdr.valid_timeout)
			{
			dat = lpvt->vAcc;
			dat *= (float)0.001;
			GPSAccData.covs.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.covs.pos_z_cov = dat;
			}
		else
			{
			GPSAccData.covs.hdr.valid_timeout = 0;
			GPSAccData.covs.pos_z_cov = 0;
			}
			
			
		bool f_fix_novalid = false;
		
		if (lpvt->flags & 1)
			{
			GPSAccData.satelitevieww.data = lpvt->numSV;
			//GPSAccData.sateliteused.data = lpvt->numSV;
			GPSAccData.modeFIX2.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.satelitevieww.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			//GPSAccData.sateliteused.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			}
		else
			{
			f_fix_novalid = true;
			}
	
		if (f_fix_novalid)
			{
			GPSAccData.satelitevieww.hdr.valid_timeout = 0;
			GPSAccData.sateliteused.hdr.valid_timeout = 0;
			GPSAccData.modeFIX2.hdr.valid_timeout = 0;
			GPSAccData.modeFIX2.data = 0;
			}
		else
			{
			unsigned char c_fix_mode = lpvt->fixType;
			if (c_fix_mode > 3 || c_fix_mode == 1) c_fix_mode = 0;
			GPSAccData.modeFIX2.data = c_fix_mode;
			}
			
		dat = lpvt->sAcc; 
		dat *= (float)0.001;
		GPSAccData.covs.velocity_x_cov = dat;
		GPSAccData.covs.velocity_y_cov = dat;
		GPSAccData.covs.velocity_z_cov = dat;
		GPSAccData.covs.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
				
		rv = true;
		}
return rv;	
}





unsigned long IGPS::crc24q_hash(unsigned char *data, unsigned short len)
{
    short i;
    unsigned long crc = 0;
    for (i = 0; i < len; i++) {
			crc = (crc << 8) ^ crc24q[data[i] ^ (unsigned char)(crc >> 16)];
			}
    crc = (crc & 0x00ffffff);
    return crc;
}



void IGPS::crc24q_sign (unsigned char *data, short len)
{
    unsigned long crc = crc24q_hash(data, len);
    data[len] = HI(crc);
    data[len + 1] = MID(crc);
    data[len + 2] = LO(crc);
}



bool IGPS::crc24q_check(unsigned char *data, short len)
{
    unsigned long crc = crc24q_hash(data, len - 3);
    return (((data[len - 3] == HI(crc)) && (data[len - 2] == MID(crc)) && (data[len - 1] == LO(crc))));
}





uint8_t IGPS::GetCurentSat_count ()
{
	return cur_sat_amount;
}



uint8_t IGPS::GetDimensModeGPS ()
{
	return gpsDmode;
}




TCANARDUM980::TCANARDUM980 (EUSARTSPD si, uint32_t asz, TSERIALUSR *obj, S_GPIOPIN *p_reset) : IGPS(si, asz, obj, p_reset)
{
	Timer_VELOCITY = 0;
	Timer_req_AGRIC = 0;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_VELOCITY);
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&Timer_req_AGRIC);
	usart_speed_ix = EUSARTSPD_115200;
	to_detect_speed ();
	f_version_req = true;
	
	cur_sat_amount = 0;
	gpsDmode = 0;
}





static const char *msgtxtarr[ERATECFGTAG_ENDENUM] = {"GPGGA","GPRMC", "GPGSA"};

void TCANARDUM980::send_nmea_cmd_set_rate (ERATECFGTAG tg, bool f_10hz)
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = msgtxtarr[tg];
str += ' ';
str += (f_10hz)?"0.1":"1";
str += "\r\n";

RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_nmea_cmd_gps_speed (uint32_t s)
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "CONFIG COM1 ";
str.Add_ULong (s);
str += ' ';
str += "8 n 1";
str += "\r\n";

RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_dops_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "STADOPB 1";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_velocity_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "BESTNAVXYZB";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_pvtsln_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "PVTSLNB 1";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_eventsln_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "EVENTSLNB 1";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());	
}



void TCANARDUM980::send_version_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "VERSIONB";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_agric_request ()
{
TSTMSTRING str(buf_tx, sizeof(buf_tx)-1);
str = "AGRICB";
str += "\r\n";
RawTransmit (buf_tx, str.Length ());
}



void TCANARDUM980::send_work_speed ()
{
usart_speed_ix = c_work_speed_ix;
send_nmea_cmd_gps_speed (C_USARTSPEED_INDX[usart_speed_ix]);
}


void TCANARDUM980::gps_hardware_reset ()
{
}



/*
void TCANARDUM980::task_rx ()
{
    uint32_t loc_push_ix, data_size;
		uint32_t rtcm_find_ix, rtcm_find_size;
		uint32_t nmea_find_ix, nmea_find_size;
		uint32_t raw_find_ix, raw_find_size;
		bool f_nmea_finded, f_rtcm_finded, f_detect;
    if (push_ix >= c_rx_buf_alloc_size) 
			{
			push_ix  = 0;
			return;
			}
	
		loc_push_ix = push_ix;
		data_size = loc_push_ix;
    while (data_size)
        {
				rtcm_find_ix = 0; rtcm_find_size = 0;
				nmea_find_ix = 0; nmea_find_size = 0;
				raw_find_ix = 0; raw_find_size = 0;
				f_detect = false;
				do  {
						f_rtcm_finded = check_rtcm_linedata (rtcm_find_ix, rtcm_find_size, loc_push_ix);
						f_nmea_finded = check_nmea_linedata (nmea_find_ix, nmea_find_size, loc_push_ix);
					
						if (f_nmea_finded && f_rtcm_finded)
							{
							if (rtcm_find_ix > nmea_find_ix)
								{
								f_rtcm_finded = false;
								}
							else
								{
								f_nmea_finded = false;
								}
							}
						if (f_rtcm_finded)
							{
							if (rtcm_find_size > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // + 1=d3, +2=len, +3=bytes crc
									{
									uint32_t l_size = rtcm_find_size - (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE);    // +3 bytes header + 3bytes crc
									uint8_t *adr = (uint8_t*)&rxdatabuf[rtcm_find_ix];
									adr += sizeof(HDRTRANPLAYRTCM3);   // +3 bytes header
									cb_rtcm3_data (adr, l_size);
									}
							raw_find_ix = rtcm_find_ix;
							raw_find_size = rtcm_find_size;
							f_detect = true;
							break;
							}
						if (f_nmea_finded)
							{
							nmea_raw_cnt++;
							if (nmea_find_size > 4)  // +$ +* +crc
									{
									uint32_t l_size = nmea_find_size - 4;
									uint8_t *adr = (uint8_t*)&rxdatabuf[nmea_find_ix];
									adr += 1;       // + 1byte '$'
									if (cb_nmea_data (adr, l_size)) 
										{
										nmea_compete_cnt++;
										loc_cnt_nmea++;   // fps counter
										f_new_gps_data = true;
										//f_new_gps_data = true;
										}
									}
							raw_find_ix = nmea_find_ix;
							raw_find_size = nmea_find_size;
							f_detect = true;
							break;
							}
						} while (false);

          if (f_detect)
              {
              // ��������� ����� �� ����������� ���������� ������ � ������
              uint32_t last_ix = raw_find_ix + raw_find_size;
							loc_cnt_messages++;
							__disable_irq ();				// critical section
							loc_push_ix = push_ix;
              if (last_ix <= loc_push_ix)
                  {
                  uint32_t tailsize = loc_push_ix - last_ix;
                  CopyMemorySDC ((char*)&rxdatabuf[last_ix], (char*)rxdatabuf, tailsize);
                  push_ix = tailsize;
									loc_push_ix = push_ix;
									data_size -= raw_find_size;
                  }
              else
                {
                // �������������� ������
                push_ix = 0;
                }
							__enable_irq ();
              }
          else
            {
            break;
            }
        }
}
*/


void TCANARDUM980::Task ()
{	
	switch (SWVal)
		{
		case EGPSSW_SPEED_SET:
			{
			if (!OtherTimer)
				{
				if (usart_speed_cnt)
					{
					if (!tx_processed ())
						{
						gps_uart_speed_set ((EUSARTSPD)C_USARTSPEED_INDX[usart_speed_ix]);
						send_nmea_cmd_gps_speed (C_USARTSPEED_INDX[c_work_speed_ix]);
						OtherTimer = 100;	
						SWVal = EGPSSW_RATE_SET;
						rate_set_ix = ERATECFGTAG_GGA;
						usart_speed_cnt--;
						}
					}
				else
					{
					gps_uart_speed_set ((EUSARTSPD)C_USARTSPEED_INDX[c_work_speed_ix]);
					SWVal = EGPSSW_WORK;
					RMC_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
					GGA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
					GSA_contrl_Timer = C_DEFMESSAGECONTROL_TIME;
					RXTraficTimeout = C_GPS_NODATA_TIMEOUT + 100;
					OtherTimer = C_MATTX_TIMEOUT;
					}
				}
			break;
			}
		case EGPSSW_RATE_SET:
			{
			if (!OtherTimer)
				{
				if (rate_set_ix < ERATECFGTAG_ENDENUM)
					{
					if (!tx_processed ())
						{
						send_nmea_cmd_set_rate ((ERATECFGTAG)rate_set_ix, true);
						rate_set_ix++;
						OtherTimer = 100;
						}
					}
				else
					{
					SWVal = EGPSSW_SPEED_SET;
					NextSpeedSearch ();
					}
				}
			break;
			}
		case EGPSSW_WORK:
			{
			rx_tasks ();		// ������� ���� ���������� � ���������
			if (loc_cnt_messages)	
				{
				loc_cnt_messages = 0;
				RXTraficTimeout = C_GPS_NODATA_TIMEOUT;		// ������� ������
				}

			if (!OtherTimer && !tx_processed ())
				{
				switch (reqsw)
					{
					case EUCREQSW_DOP:
						{
						if (!Timer_req_DOP)
							{
							send_dops_request ();	
							OtherTimer = C_MATTX_TIMEOUT;
							Timer_req_DOP = C_PERIOD_DOP;
							}
						reqsw = EUCREQSW_AGRIC;
						break;
						}
					case EUCREQSW_AGRIC:
						{
						if (!Timer_req_AGRIC)
							{
							send_agric_request ();	
							OtherTimer = C_MATTX_TIMEOUT;
							Timer_req_AGRIC = C_PERIOD_AGRIC;
							}
						reqsw = EUCREQSW_VELOCITY;
						break;
						}
					case EUCREQSW_VELOCITY:
						{
						if (!Timer_VELOCITY)
							{
							send_velocity_request ();
							//send_pvtsln_request ();	
							//send_eventsln_request ();
							OtherTimer = C_MATTX_TIMEOUT;
							Timer_VELOCITY = C_PERIOD_VELOCITY;
							}
						if (!f_version_req)
							{
							reqsw = EUCREQSW_DOP;
							}
						else
							{
							reqsw = EUCREQSW_VERSION;
							}
						break;
						}
					case EUCREQSW_VERSION:
						{
						send_version_request ();
						f_version_req = false;
						OtherTimer = C_MATTX_TIMEOUT;
						reqsw = EUCREQSW_DOP;
						break;
						}
					default:
						{
						reqsw = EUCREQSW_DOP;
						OtherTimer = 0;
						break;
						}
					}
				}
			if (!RMC_contrl_Timer || !GGA_contrl_Timer || !GSA_contrl_Timer) 
				{
				to_detect_speed ();
				}
			if (!RXTraficTimeout) 
				{
				to_detect_speed ();
				}
			break;
			}
		case EGPSSW_RESET:
			{
			gps_power_pin (false);
			OtherTimer = 150; // 120
			SWVal = EGPSSW_RESET_WAIT;
			// ��� break
			}						
		case EGPSSW_RESET_WAIT:
			{
			if (!OtherTimer)
				{
				gps_power_pin (true);
				to_detect_speed ();
				}
			break;
			}
		default:
			{
			break;
			}
		}
}



bool TCANARDUM980::detect_protocols (uint32_t start_ix, uint32_t stop_ix)
{
	bool rv = false;
  uint32_t find_ix, find_size;
	
	if (stop_ix > start_ix)
		{
		if (check_rtcm_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_RTCM].type = EDTPROTOCOL_RTCM;
			dtct_tags[EDTPROTOCOL_RTCM].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_RTCM].ix_start = find_ix;
			rv = true;
			}
			
		if (check_nmea_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_NMEA].type = EDTPROTOCOL_NMEA;
			dtct_tags[EDTPROTOCOL_NMEA].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_NMEA].ix_start = find_ix;
			rv = true;
			}
			
		if (check_um_linedata (find_ix, find_size, start_ix, stop_ix))
			{
			dtct_tags[EDTPROTOCOL_UM].type = EDTPROTOCOL_UM;
			dtct_tags[EDTPROTOCOL_UM].fr_size = find_size;
			dtct_tags[EDTPROTOCOL_UM].ix_start = find_ix;
			rv = true;
			}
		}
return rv;
}



bool TCANARDUM980::parse_protocols (const S_DTPROTOCOL_T &d)
{
	bool rv = false;
	uint32_t find_size = d.fr_size;
	//uint32_t find_ix = d.ix_start;
	uint8_t *adr = (uint8_t*)&rxdatabuf[d.ix_start];
	
	switch (d.type)
		{
		case EDTPROTOCOL_NMEA:
			{
			if (find_size > 4)
				{
				uint32_t l_size = find_size - 4;
				adr += 1;       // + 1byte '$'
				rv = cb_nmea_data (adr, l_size);
				if (rv) f_new_gps_data = true;
				}
			break;
			}
		case EDTPROTOCOL_RTCM:
			{
			rv = cb_rtcm3_data (adr, find_size);
			break;
			}
		case EDTPROTOCOL_UM:
			{
			rv = cb_um_data ((S_UMHEADER_T*)adr, find_size);
			if (rv) f_new_gps_data = true;
			break;
			}
		default: break;
		}
	return rv;
}




bool TCANARDUM980::cb_um_data (S_UMHEADER_T *lUm, uint32_t size)
{
bool rv = false;
if (lUm)
	{
	char *lpMessUM = ((char*)lUm) + sizeof (S_UMHEADER_T);
	switch (lUm->mess_id)
		{
		case C_UMID_VERSION:
			{
			S_UMVERSIONS_T *versframe = (S_UMVERSIONS_T*)lpMessUM;
			versframe->sw_version_str[sizeof(versframe->sw_version_str)-1] = 0;
			break;
			}
		case C_UMID_STADOP:
			{
			S_UMDOP_T *dopframe = (S_UMDOP_T*)lpMessUM;
			GPSAccData.PDOP.value = dopframe->Pdop;
			GPSAccData.VDOP.value = dopframe->Vdop;
			GPSAccData.HDOP.value = dopframe->Hdop;
			GPSAccData.EDOP.value = dopframe->Edop;
			GPSAccData.NDOP.value = dopframe->Ndop;
			GPSAccData.TDOP.value = dopframe->Tdop;
			GPSAccData.GDOP.value = dopframe->gdop;
			//GPSAccData.satelitevieww.data = dopframe->prn;
			GPSAccData.PDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.HDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.EDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.NDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.TDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.GDOP.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			//GPSAccData.satelitevieww.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			//f_new_aux_data = rv;
			break;
			}
		/*
		case C_UMID_PVTSLN:
			{
			S_UMPVTSL_T *pvt = (S_UMPVTSL_T*)lpMessUM;
				
			GPSAccData.VELN.value_m_s = pvt->psrvel_north;
			GPSAccData.VELE.value_m_s = pvt->psrvel_east;
			GPSAccData.VELD.value_m_s = 0;
				
			GPSAccData.VELN.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELE.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELD.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			f_new_aux_data = rv;
			break;
			}
		case C_UMID_AGRIC:
			{
			break;
			}
		*/
		/*
		case C_UMID_EVENTSLN:
			{
			S_UMEVENTSLN_T *sln = (S_UMEVENTSLN_T*)lpMessUM;
			GPSAccData.VELN.value_m_s = sln->northVel;
			GPSAccData.VELE.value_m_s = sln->EastVel;
			GPSAccData.VELD.value_m_s = sln->upVel;
				
			GPSAccData.VELN.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELE.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELD.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			f_new_aux_data = rv;
			break;
			}
		*/
		case C_UMID_AGRIC:
			{
			S_UMAGRIC_T *agric = (S_UMAGRIC_T*)lpMessUM;
			GPSAccData.VELE.value_m_s = agric->East_velocity_m_s;
			GPSAccData.VELN.value_m_s = agric->North_velocity_m_s;
			GPSAccData.VELD.value_m_s = agric->Up_velocity_m_s * -1.0F;
			GPSAccData.VELN.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELE.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.VELD.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			//f_new_aux_data = rv;
			break;
			}
		case C_UMID_BESTNAVXYZ:
			{
			S_UMBESTNAVXYZ_T *nav = (S_UMBESTNAVXYZ_T*)lpMessUM;
			//GPSAccData.VELN.value_m_s = nav->V_y * -1.0;
			//GPSAccData.VELE.value_m_s = nav->V_x * -1.0;
			//GPSAccData.VELD.value_m_s = nav->V_z * -1.0;
			//GPSAccData.VELN.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			//GPSAccData.VELE.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			//GPSAccData.VELD.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.satelitevieww.data = nav->sat_track;
			GPSAccData.sateliteused.data = nav->sat_used;	
				
			GPSAccData.covs.pos_x_cov = nav->P_x_q;
			GPSAccData.covs.pos_y_cov = nav->P_y_q;
			GPSAccData.covs.pos_z_cov = nav->P_z_q;
			GPSAccData.covs.velocity_x_cov = nav->V_x_q;
			GPSAccData.covs.velocity_y_cov = nav->V_y_q;
			GPSAccData.covs.velocity_z_cov = nav->V_z_q;
			GPSAccData.covs.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;	
			GPSAccData.sateliteused.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			GPSAccData.satelitevieww.hdr.valid_timeout = C_DEF_MESSAGE_TIMEOUT;
			rv = true;
			//f_new_aux_data = rv;
			break;
			}
		
		default:
			{
			break;
			}
		}
	}
return rv;
}
