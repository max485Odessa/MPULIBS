#include <vcl.h>
#include "TJSON.HPP"
#include "textrut.hpp"




TLPObj::TLPObj ()
{
Clear();
}



bool TJSON::Save_JSON (TDString flname)
{
bool rv = false;
TMCreateWrireStream Wr1;
if (JSON_data != "" && flname != "")
    {
//    String utf8 = F1251ToUTF8 (JSON_data);
    TDString utf8 = JSON_data;//F1251ToUTF8 (JSON_data);
    char *lram = (char*)utf8.c_str();
    unsigned long sz = utf8.length();
    SetCurrentDir(JsonFileDirPath.c_str());
    if (Wr1.CreateStream((char*)flname.c_str()))
        {
        if (Wr1.WriteBlock(lram, sz) == sz) rv = true;
        Wr1.CloseStream();
        }
    }
return rv;
}



bool TJSON::Load_JSON (char *flname)
{
bool rv = false;
TMCreateReadStream Rd1;
if (flname)
    {
    JsonFileDirPath = GetExecPath ();
    if (Rd1.OpenStream(flname))
        {
        unsigned int sizes = Rd1.GetFileSize ();
        if (sizes)
            {
            unsigned char *lpRam = new unsigned char [(sizes + 8)];
            if (Rd1.ReadData(lpRam, sizes) == sizes)
                {
                CreateStringFromLine_cnt (JSON_data, lpRam,sizes);
                //JSON_data = UTF8To1251 (JSON_data);
                //JSON_data = UTF8To1251 (JSON_data);
                UpdateLP ();
                rv = true;
                }
            delete [] lpRam;
            }
        Rd1.CloseStream();
        }
    }
return rv;
}



TDString TJSON::Get_JSON ()
{
return JSON_data;
}



void TLPObj::Clear()
{
lStart = 0;
lStop = 0;
f_is_array = false;
}



void TLPObj::Start (char *adr)
{
lStart = adr;
lStop = adr;
}



void TLPObj::TwoCuteTxt ()
{
// ������� ��������� �������
while (lStart < lStop)
    {
    if (*lStart != ' ') break;
    lStart++;
    }
lStop--;
// ������� ������� � �����
while (lStop > lStart)
    {
    if (*lStop != ' ') break;
    lStop--;
    }
}



long TLPObj::Size ()
{
long rv = 0;
if (IsCorrect ()) rv = lStop - lStart;
return rv;
}



bool TLPObj::IsCorrect ()
{
bool rv = false;
if (lStop !=0 && lStart != 0 && lStop >= lStart) rv = true;
return rv;
}



TJSON::TJSON ()
{
Clear ();
}



void TJSON::Clear ()
{
CurentPath = "";
JSON_data = "";
BaseJSN_TLP.Clear();
}



TJSON::~TJSON ()
{
	
}



void TJSON::SetJSON_data (TDString strr)
{
JSON_data = strr;
UpdateLP ();
p_GetObj_size (ljson_data, '{', BaseJSN_TLP);   // ������������� ������� ����� �� �������� ������� ������
}


void TJSON::UpdateLP ()
{
ljson_data = (char*)JSON_data.c_str();
OpenPath (CurentPath);
}



bool TJSON::GetItem (TDString const &ItmemName, TDString &d_ItemVal, EJSNTYPE *lpd_type)
{
bool rv = false;
TLPObj outParam;
EJSNTYPE tmp_type;
long indx = FindItemParam_fromOBJ (0, ItmemName, outParam, &tmp_type);
if (indx >= 0)
    {
    d_ItemVal = TLP_to_Str (outParam);
    if (lpd_type) *lpd_type = tmp_type;
    rv = true;
    }
return rv;
}



EJSEXTYPE TJSON::GetTypeString (const TDString &name)
{
EJSEXTYPE rv = EJSEXTYPE_NONE;
do  {
    if (CheckDecValue (name))
        {
        unsigned long sz = GetLenStr ((char*)name.c_str());
        if (sz > 9)
            {
            rv = EJSEXTYPE_DIG_LONG64;
            }
        else
            {
            rv = EJSEXTYPE_DIG_LONG32;
            }
        break;
        }
    if (CheckFloatValue (name))
        {
        rv = EJSEXTYPE_DIG_FLOAT;
        break;
        }
    } while (false);
return rv;
}



bool TJSON::GetItemExtType (TDString const &ItmemName, TDString &d_ItemVal, EJSEXTYPE *d_type)
{
bool rv = false;

    EJSNTYPE ctype;
    EJSEXTYPE nwext_type = EJSEXTYPE_NONE;
    if (GetItem (ItmemName, d_ItemVal, &ctype))
        {
        switch (ctype)
            {
            case EJST_DIG:
                {
                nwext_type = GetTypeString (d_ItemVal);
                break;
                }
            case EJST_STRING:
                {
                nwext_type = EJSEXTYPE_STRING;
                break;
                }
            case EJST_OBJ:
                {
                nwext_type = EJSEXTYPE_OBJ;
                break;
                }
            case EJST_ARRAY:
                {
                nwext_type = EJSEXTYPE_OBJ;
                break;
                }
            }
        if (nwext_type != EJSEXTYPE_NONE)
            {
            if (d_type) *d_type = nwext_type;
            rv = true;
            }
        }

return rv;
}



void TJSON::CuteCommaStr (TDString &JSON_strparam)
{
TDString rv = "";
char *lpRam = (char*)JSON_strparam.c_str();
char dat;
long ofs_start = -1;
long ofs_stop = -1;
unsigned long indx = 0;
while (true)
    {
    dat = lpRam[0];
    if (!dat) break;
    if (dat == '\"')
        {
        if (ofs_start == -1)
            {
            ofs_start = indx;
            }
        else
            {
            ofs_stop = indx;
            }
        }
    indx++;
    lpRam++;
    }
if (ofs_stop > ofs_start)
    {
    rv = JSON_strparam.substr ((ofs_start + 1), (ofs_stop - ofs_start - 1));
    JSON_strparam = rv;
    }
}



bool TJSON::Get_Str (TDString const &name, TDString *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_STRING)
    {
    if (lpvalue)
        {
        CuteCommaStr (d_ItemVal);

        // ������� ��������� ����������� ��������
        shield_set_to (d_ItemVal, false);
        *lpvalue = d_ItemVal;

        }
    rv = true;
    }
return rv;
}



bool TJSON::Get_Bool (TDString const &name, bool *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_DIG)
    {
    if (d_ItemVal == "true" || d_ItemVal == "TRUE")
        {
        rv = true;
        if (lpvalue) *lpvalue = true;
        }
    else if (d_ItemVal == "false" || d_ItemVal == "FALSE")
        {
        rv = true;
        if (lpvalue)*lpvalue = false;
        }
    }
return rv;
}



bool TJSON::Get_Numb (TDString const &name, long *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_DIG)
    {
    long dat;
    unsigned char *rslt = ConvertStrToLong ((unsigned char*)d_ItemVal.c_str(), &dat);
    if (rslt)
        {
        if (lpvalue) *lpvalue = dat;
        rv = true;
        }
    }
return rv;
}



bool TJSON::Get_Numb_LL (TDString const &name, long long *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_DIG)
    {
    int64_t dat;
    unsigned char *rslt = ConvertStrToLong64 ((unsigned char*)d_ItemVal.c_str(), &dat);
    if (rslt)
        {
        if (lpvalue) *lpvalue = dat;
        rv = true;
        }
    }
return rv;
}



bool TJSON::Get_Numb_F (TDString const &name, float  *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_DIG)
    {
    if (lpvalue)
        {
        unsigned long lenn = GetLenS((char*)d_ItemVal.c_str());
        float val_f;
        if (TxtToFloat (&val_f, (char*)d_ItemVal.c_str(), lenn))
            {
            if (lpvalue) *lpvalue = val_f;
            }
        }
    rv = true;
    }
return rv;
}



bool TJSON::Get_Arr (TDString const &name, TDString *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_ARRAY)
    {
    if (lpvalue) *lpvalue = d_ItemVal;
    rv = true;
    }
return rv;
}



bool TJSON::Get_Obj (TDString const &name, TDString *lpvalue)
{
bool rv = false;
EJSNTYPE pd_type;
TDString d_ItemVal = "";
bool tslt = GetItem (name, d_ItemVal, &pd_type);
if (tslt && pd_type == EJST_OBJ)
    {
    if (lpvalue) *lpvalue = d_ItemVal;
    rv = true;
    }
return rv;
}






long TJSON::GetCurObj_size ()
{
long rv = 0;
if (BaseJSN_TLP.IsCorrect())
    {
    char delim = '{';
    if (BaseJSN_TLP.f_is_array) delim = '[';
    TLPObj outtlp;
    rv = p_GetObj_size (BaseJSN_TLP.lStart, delim, outtlp);
    }
return rv;
}




TParsJSNKey::TParsJSNKey()
{
jsSeqKey_find = "";
}



bool TParsJSNKey::p_parsekey_getFist (TDString const &JS_key, TDString &d_Name, EJSKEYTYPE &d_type, long &d_indx)
{
jsSeqKey_find = JS_key;
jslSqKey = (char*)jsSeqKey_find.c_str();
jsIndx = 0;
return p_parsekey_getNext (d_Name, d_type, d_indx);
}


// �������� ������ json ���� ������:  NAME.NAME,NAME[101].ADRESS.POINT[1]
// ��������� ��� � ��� ���������� ������� 
// ��� ����: ��������, ������, ������. ��� ������� ������ �������� - ������
bool TParsJSNKey::p_parsekey_getNext (TDString &d_Name, EJSKEYTYPE &d_type, long &d_indx)
{
bool rv = false;
TDString Str_rslt = "";
TDString str_indx = "";
char dat;
bool F_isPoint = false;
bool F_isArr = false;   // ��������� � ��������� ���������
bool F_data = false;    // ��� ���������� � ������������
bool f_arr_detect = false;      // ������ ���������
if (jslSqKey)
    {
    while (true)
        {
        dat = jslSqKey[0];
        if (dat != 10 && dat != 13 && dat != 9)
            {
            if (!dat)
                {
                jslSqKey = 0;
                break;
                }
            if (dat == '.')
                {
                F_isPoint = true;
                jslSqKey++;
                break;
                }
            else
                {
                if (dat == '[')
                    {
                    f_arr_detect = true;
                    str_indx = "";
                    F_isArr = true;
                    }
                else
                    {
                    if (dat == ']')
                        {
                        F_isArr = false;
                        }
                    else
                        {
                        if (F_isArr)
                            {
                            str_indx += dat;
                            //DigCount++;
                            }
                        else
                            {
                            Str_rslt += dat;
                            F_data = true;
                            }
                        }
                    }
                }
            }
        jslSqKey++;
        }
    if (F_data)
        {
        if (f_arr_detect)
            {
            // ������ �������� ��� �������� �������
            if (str_indx != "")
                {
                long dat;
                unsigned char *rslt = ConvertStrToLong ((unsigned char *)str_indx.c_str(), &dat);
                if (rslt)
                    {
                    d_indx = dat;
                    d_type = EJSKEY_INDEXES;
                    rv = true;
                    }
                }
            else
                {
                // �������� ���� � ������� ��� �������� ������� ��������� !
                // ������������� ��������� ��������� �������������� ������� �� ��������� 0
                d_indx = 0;
                d_type = EJSKEY_INDEXES;
                rv = true;
                }
            }
        else
            {
            // ������ ��� ��������
            if (Str_rslt != "")
                {
                d_type = EJSKEY_SINGLE;
                rv = true;
                }
            }
        d_Name = Str_rslt;
        }
    }
return rv;
}

/*
bool TParsJSNKey::p_parsekey_getNext (String &d_Name, EJSKEYTYPE &d_type, long &d_indx)
{
bool rv = false;
String Str_rslt = "";
String str_indx = "";
char dat;
bool F_isPoint = false;
bool F_isArr = false;   // ��������� � ��������� ���������
bool F_data = false;    // ��� ���������� � ������������
long DigCount = 0;      // ������ ���������
if (jslSqKey)
    {
    while (true)
        {
        dat = jslSqKey[0];
        if (dat != 10 && dat != 13 && dat != 9)
            {
            if (!dat)
                {
                jslSqKey = 0;
                break;
                }
            if (dat == '.')
                {
                F_isPoint = true;
                jslSqKey++;
                break;
                }
            else
                {
                if (dat == '[')
                    {
                    DigCount = 0;
                    str_indx = "";
                    F_isArr = true;
                    }
                else
                    {
                    if (dat == ']')
                        {
                        F_isArr = false;
                        }
                    else
                        {
                        if (F_isArr)
                            {
                            str_indx += dat;
                            DigCount++;
                            }
                        else
                            {
                            Str_rslt += dat;
                            F_data = true;
                            }
                        }
                    }
                }
            }
        jslSqKey++;
        }
    if (F_data)
        {
        if (DigCount)
            {
            // ������ �������� ��� �������� �������
            if (str_indx != "")
                {
                d_indx = str_indx.ToInt();
                d_type = EJSKEY_ARR;
                rv = true;
                }
            }
        else
            {
            // ������ ��� ��������
            if (Str_rslt != "")
                {
                d_type = EJSKEY_OBJ;
                rv = true;
                }
            }
        d_Name = Str_rslt;
        }
    }
return rv;
}
*/




bool TJSON::GetItem_RamPos_inx (TLPObj *IN_Tlp, long i_array, JSONARRITEMPAR &lpJS_par)
{
if (!IN_Tlp) IN_Tlp = &BaseJSN_TLP;
if (!IN_Tlp->IsCorrect()) return false;

bool rv = false;
char *lRam = IN_Tlp->lStart;
unsigned long sizes = IN_Tlp->Size() + 1;
if (sizes)
    {
    char datt;
    char type_el = 0;
    unsigned long count_char_m = 0;
    unsigned long count_char_o = 0;
    unsigned long count_char = 0;
    lpJS_par.item_amount = 0;
    lpJS_par.prevcoma_ofs = -1;
    long comalast_ofs = -1;
    bool F_sub_entered = false;
    bool F_is_item = false;
    bool f_is_error = false;
    char delim_dat = 0;
    bool F_is_comma_body = false;
    bool F_comma_slash = false;
    unsigned char skip_counter = 0;
    char end_char = -1;
    lpJS_par.global_ofs = IN_Tlp->lStart - ljson_data;
    lpJS_par.end_ofs = -1;

    while (count_char < sizes)
        {
        datt = lRam[0];
        if (!datt) break;


        if (F_sub_entered)  // ����� � ������ ��� ������
            {
            if (F_is_item)  // ����� � ������� ������� ��� �������
                {
                bool f_need_check = false;


                if (!F_is_comma_body)
                    {
                    if (datt == '{') count_char_o++;
                    if (datt == '[') count_char_m++;
                    if (datt == '\"') F_is_comma_body = true; // ���� � ���� ��������� ����������
                    }
                else
                    {
                    // ��������� � �������� ���������
                    if (!F_comma_slash)
                        {
                        switch (datt)
                            {
                            case '\"':
                                {
                                F_is_comma_body = false;    // ������� � ����� ���������
                                skip_counter = 1;
                                break;
                                }
                            case '\\':
                                {
                                F_comma_slash = true;
                                skip_counter = 1;
                                break;
                                }
                            }
                        }
                    else
                        {
                        // ������� ���� ����� ��������
                        switch (datt)
                            {
                            case '/':
                            case 'b':
                            case 'f':
                            case 'n':
                            case 'r':
                            case 't':
                            case '\\':
                            case '\"':
                                {
                                skip_counter = 1;
                                break;
                                }
                            case 'u':
                                {
                                skip_counter = 4;
                                break;
                                }
                            }
                        F_comma_slash = false;  // ������� ������� �����������
                        }
                    }


                if (!skip_counter)
                    {
                    if (!F_is_comma_body)
                        {
                        switch (datt)
                            {
                            case '}':
                                {
                                if (count_char_o)
                                    {
                                    count_char_o--;
                                    }
                                else
                                    {
                                    if (end_char == datt)
                                        {
                                        if (lpJS_par.stop_ofs == - 1) lpJS_par.stop_ofs = count_char - 1;
                                        lpJS_par.end_ofs = count_char;     // ������ ����� ������  +
                                        if ((lpJS_par.item_amount-1) != i_array) return false;
                                        return true;
                                        }
                                    else
                                        {
                                        // ������ - ������ �� ��� ������ ����������
                                        f_is_error = true;
                                        }
                                    }
                                break;
                                }
                            case ']':
                                {
                                if (count_char_m)
                                    {
                                    count_char_m--;
                                    }
                                else
                                    {
                                    if (end_char == datt)
                                        {
                                        if (lpJS_par.stop_ofs == - 1) lpJS_par.stop_ofs = count_char - 1;
                                        lpJS_par.end_ofs = count_char;     // ������ ����� ������  +
                                        if ((lpJS_par.item_amount-1) != i_array) return false;
                                        return true;
                                        }
                                    else
                                        {
                                        // ������ - ������ �� ��� ������ ����������
                                        f_is_error = true;
                                        }
                                    }
                                break;
                                }
                            case ',':
                                {
                                if (!count_char_m && !count_char_o)
                                    {
                                    lpJS_par.stop_ofs = count_char - 1;
                                    lpJS_par.coma_ofs = count_char;
                                    comalast_ofs = count_char;
                                    f_need_check = true;

                                    }
                                else
                                    {
                                    // ������� � ���� ������� ��� �������
                                    }
                                break;
                                }
                            default:
                                {
                                if (datt > ' ')
                                    {
                                    if (lpJS_par.start_ofs == -1) lpJS_par.start_ofs = count_char;
                                    }
                                break;
                                }
                            }
                        }
                    //if (f_is_error || (lpJS_par.item_amount-1) == i_array) break;
                    if (f_need_check)
                        {
                        if ((lpJS_par.item_amount-1) >= i_array)
                            {
                            break;
                            }
                        else
                            {
                            lpJS_par.start_ofs = -1;
                            lpJS_par.stop_ofs = -1;
                            lpJS_par.coma_ofs = -1;
                            lpJS_par.end_ofs = -1;
                            F_is_item = false;
                            }
                        //if ((lpJS_par.item_amount-1) > i_array) break;
                        }
                    }
                else
                    {
                    skip_counter--; // ������� ���� �������
                    }
                if (f_is_error) break;
                }
            else
                {
                if (datt > ' ')
                    {
                    lpJS_par.start_ofs = -1;
                    lpJS_par.stop_ofs = -1;
                    lpJS_par.coma_ofs = -1;
                    lpJS_par.end_ofs = -1;
                    if (datt != end_char)
                        {
                        count_char_m = 0;
                        count_char_o = 0;
                        if (datt == '[')count_char_m++;
                        if (datt == '{')count_char_o++;
                        if (datt == '\"') F_is_comma_body = true;
                        lpJS_par.prevcoma_ofs = comalast_ofs;
                        lpJS_par.start_ofs = count_char;
                        delim_dat = datt;
                        F_is_item = true;
                        lpJS_par.item_amount++;        // ������� ���������� ���������
                        //cur_item++;
                        }
                    else
                        {
                        break;
                        }
                    }
                }
            }
        else
            {
            // end_char
            bool f_ent = false;
            switch (datt)
                {
                case '[':
                    {
                    end_char = ']';
                    f_ent = true;
                    break;
                    }
                case '{':
                    {
                    end_char = '}';
                    f_ent = true;
                    break;
                    }
                }
            if (f_ent)
                {
                lpJS_par.m_enter_ofs = count_char;
                F_sub_entered = true;
                }
            }
        lRam++;
        count_char++;
        }
    rv = !f_is_error;
    }
return rv;
}



TLPObj TJSON::Item_get (TLPObj *IN_Tlp, long i_array)
{
if (!IN_Tlp) IN_Tlp = &BaseJSN_TLP;
TLPObj rv;// = *IN_Tlp;// = BaseJSN_TLP;        // 11.05.21
if (IN_Tlp->IsCorrect())
    {
    bool rsltf;
    JSONARRITEMPAR par;
    if (i_array >= 0)
        {
        rsltf = GetItem_RamPos_inx (IN_Tlp, i_array, par);
        if (rsltf)
            {
            if (par.stop_ofs != -1)
                {
                if (par.stop_ofs >= par.start_ofs)
                    {
                    rv.lStart = IN_Tlp->lStart + par.start_ofs;
                    rv.lStop = rv.lStart + (par.stop_ofs - par.start_ofs + 1);
                    }
                }
            }
        }
    }
return rv;
}



long TJSON::GetCurObj_ItemAmount ()
{
long indx = -1;
    JSONARRITEMPAR par;
    GetItem_RamPos_inx (0, 0xFFFFFF, par);
    indx = par.item_amount;
return indx;
}




void TJSON::shield_set_to (TDString &str, bool f_on)
{
TDString in_str = str;
TDString out_str = "";
bool f_change = false;
if (f_on)
    {
    // ������������� ��������� ������
    unsigned long ix = 0;
    unsigned long sz_end = str.length();
    char *lInp = (char*)str.c_str();
    char dta;
    while (ix < sz_end)
        {
        dta = *lInp++;
        if (dta == '\\')
            {
            out_str += dta;
            f_change = true;
            }
        out_str += dta;
        ix++;
        }
    }
else
    {
    // ���������� ��������� ������
    unsigned long ix = 0;
    unsigned long sz_end = str.length();
    char *lInp = (char*)str.c_str();
    char dta;
    bool f_skip = false;
    while (ix < sz_end)
        {
        dta = *lInp++;
        if (!f_skip)
            {
            if (dta == '\\')
                {
                ix++;
                f_skip = true;      // ���������� ������ ����, ��������� ��������� ������������
                f_change = true;    // ������ ���������
                continue;
                }
            }
        out_str += dta;
        f_skip = false;
        ix++;
        }
    }
if (f_change) str = out_str;
}


// ������ �������� ������ ��������� �������� ���������, ��: ���, ��������, ��� ���������
// ���� ������� ������ �� ������� - ���������� ������� �����
EJSNTYPE TJSON::ExtractTypeParam (TLPObj *IN_Tlp, TDString &outname, TLPObj &outval)
{
EJSNTYPE rv = EJST_NONE;
if (!IN_Tlp) IN_Tlp = &BaseJSN_TLP;
if (!IN_Tlp->IsCorrect()) return rv;
char *lJsIn = IN_Tlp->lStart;;
unsigned long count = IN_Tlp->Size();
char dat;
bool F_is_comma_body = false;
char sw_enum = 0;   // 0 - ����� �����, 1 - � �����, 2 - ����� ����������� ���������, 3 - ����� ���������, 4- � ���������
bool F_is_param = false;
bool F_comma_slash = false;
long skip_counter = 0;
char code_data = -1;
bool f_exit = false;

while (count)
    {
    dat = lJsIn[0];
    if (!dat) break;

    switch (sw_enum)
        {
        case 0:     // ����� ����� ���������
            {
            if (dat == '\"')
                {
                sw_enum = 1;
                outname = "";
                }
            break;
            }
        case 1:     // � ����� ���������
            {
            if (!skip_counter)
                {
                if (!F_comma_slash)
                    {
                    switch (dat)
                        {
                        case '\"':
                            {
                            sw_enum = 2;    // ����� ���������
                            break;
                            }
                        case '\\':
                            {
                            F_comma_slash = true;       // ��� break
                            }
                        default:
                            {
                            outname += dat;
                            break;
                            }
                        }
                    }
                else
                    {
                    //
                    switch (dat)
                        {
                        case '/':
                        case 'b':
                        case 'f':
                        case 'n':
                        case 'r':
                        case 't':
                        case '\\':
                        case '\"':
                            {
                            skip_counter = 1;
                            break;
                            }
                        case 'u':
                            {
                            skip_counter = 4;
                            break;
                            }
                        }
                    F_comma_slash = false;  // ������� ������� �����������
                    outname += dat;
                    }
                }
            else
                {
                skip_counter--;
                outname += dat;
                }

            break;
            }
        case 2:     // ����� ����������� ���������
            {
            if (dat == ':')
                {
                //outval.Start((lJsIn + 1)) ;     //
                outval.Clear();
                sw_enum = 3;
                }
            break;
            }
        case 3:     // ����� ���������
            {
            if (dat > ' ')
                {
                switch (dat)
                    {
                    case '\"':
                    case '{':
                    case '[':
                        {
                        code_data = dat;
                        break;
                        }
                    default:
                        {
                        code_data = 0;
                        break;
                        }
                    }
                outval.Start(lJsIn);
                //outval.lStop++;// += dat;
                sw_enum = 4;    // � ���������
                }
            break;
            }
        case 4:     // � ���������
            {
            switch (code_data)
                {
                case 0:         // ��� break
                    {
                    if (dat == ' ')
                        {
                        f_exit = true;
                        break;
                        }
                    }
                default:
                    {
                    outval.lStop++;
                    //outval += dat;
                    break;
                    }
                }
            break;
            }
        }

    if (f_exit) break;
    lJsIn++;
    count--;
    }
if (sw_enum == 4)
    {
    switch (code_data)
        {
        case 0:
            {
            rv = EJST_DIG;
            break;
            }
        case '\"':
            {
            rv = EJST_STRING;
            break;
            }
        case '{':
            {
            rv = EJST_OBJ;
            break;
            }
        case '[':
            {
            rv = EJST_ARRAY;
            break;
            }
        }
    }
return rv;
}


// ����������� ������ �� ���������� � STRING
TDString TJSON::TLP_to_Str (TLPObj const &tlp)
{
TDString rv = "";
//if (tlp.IsCorrect()) CreateStringFromLine_cnt (rv, tlp.lStart, tlp.Size()+1);   1.07.21
if (tlp.IsCorrect()) CreateStringFromLine_cnt (rv, tlp.lStart, tlp.Size() + 1);
return rv;
}


// ������� ���� �� ����� � ���������� ��� ��� � ����� ����������
long TJSON::FindItemParam_fromOBJ (TLPObj *IN_Tlp, TDString const &nameparam, TLPObj &outTlp, EJSNTYPE *lpd_type)     // String &outParam
{
long rv = -1;
if (!IN_Tlp) IN_Tlp = &BaseJSN_TLP;
if (IN_Tlp->IsCorrect() && nameparam != "")
    {
    TDString find_name;
    TLPObj find_param;
    unsigned long indx = 0;
    bool rslt;
    TLPObj tlp1;
    if (lpd_type) *lpd_type = EJST_NONE;
    while (true)
        {
        tlp1 = Item_get (IN_Tlp, indx);
        if (tlp1.IsCorrect())
            {
            EJSNTYPE type_rv = ExtractTypeParam (&tlp1, find_name, find_param);
            if (find_name == nameparam && type_rv != EJST_NONE && find_param.IsCorrect())
                {
                if (lpd_type) *lpd_type = type_rv;
                outTlp = find_param;
                rv = indx;
                break;
                }
            }
        else
            {
            // ����� ������
            break;
            }
        indx++;
        }
    }
return rv;
}



long TJSON::Item_Find (TDString const &jsn_rawlineitem)
{
TLPObj outTlp;
long rv = FindItemParam_fromOBJ (0, jsn_rawlineitem, outTlp, 0);
return rv;
}



// ������������� PATHKEY � ������� (��������� - ������ ������� ������/������ ������� ��������� BaseJSN_TLP)
bool TJSON::OpenPath (TDString strkey)
{
bool rv = false;
if (strkey != "")
    {
    TDString d_Name = "";
    EJSKEYTYPE d_type;
    long d_indx = 0; bool f_path_ok = false;
    p_GetObj_size (ljson_data, '{', BaseJSN_TLP);   // ������������� ��������� ����� �� ����� ������� json
    bool rslt = pars_f1.p_parsekey_getFist (strkey, d_Name, d_type, d_indx);
    while (rslt)
        {
        switch (d_type)
            {
            case EJSKEY_SINGLE:     // ��� �������� ��������� ���������, ������� ��������� ��������������� �� ����� ������ ��� ������ (� ������ �������� ������� ����� �������� ������ raw item �������)
                {
                TLPObj outTlp;
                EJSNTYPE d_type_a;
                long indx = FindItemParam_fromOBJ (0, d_Name, outTlp, &d_type_a);
                if (indx >= 0 && outTlp.IsCorrect())
                    {
                    if (d_type_a == EJST_OBJ || d_type_a == EJST_ARRAY)
                        {
                        // � ������ ���������� ������� - ������������� ����� ��������� �����
                        outTlp.f_is_array = false;
                        if (d_type_a == EJST_ARRAY) outTlp.f_is_array = true;
                        rslt = SetPathKey (outTlp);
                        f_path_ok = true;
                        }
                    else
                        {
                        f_path_ok = false;
                        }
                    }
                else
                    {
                    f_path_ok = false;
                    }
                break;
                }
            case EJSKEY_INDEXES:
                {
                TLPObj outTlp;              // ������ ������� ��������� �������, ������������ �� ���� ���������
                EJSNTYPE d_type_a;
                long indx = FindItemParam_fromOBJ (0, d_Name, outTlp, &d_type_a);
                if (indx >= 0 && d_type_a == EJST_ARRAY && outTlp.IsCorrect())
                    {
                    // �������� �� : ������ ������, ������ �������� ��� "������ ��������" ����������� ������
                    TLPObj tlp;             // ������ ������� ��������� �������� �������� ��� �������� ���� �����
                    tlp = Item_get (&outTlp, d_indx);
                    if (tlp.IsCorrect())
                        {
                        tlp.f_is_array = true;
                        //if (c_type == EJST_OBJ) tlp.f_is_array = false;
                        rslt = SetPathKey (tlp);     // ��������� ����� ������� ��������� �� ������ ��� ������

                        f_path_ok = true;
                        /*
                        // item �������
                        String outname;
                        TLPObj nwview;      // ������ ������� ��������� ��������
                        EJSNTYPE c_type = ExtractTypeParam (&tlp, outname, nwview); // ��������� ��� �������� �������
                        if (c_type == EJST_OBJ || c_type == EJST_ARRAY)
                            {
                            // ������� ��������� �������� ������ ����� ���������� ������� �������� ������� ��� �������
                            if (nwview.IsCorrect())
                                {
                                tlp.f_is_array = true;
                                if (c_type == EJST_OBJ) tlp.f_is_array = false;
                                rslt = SetPathKey (tlp);     // ��������� ����� ������� ��������� �� ������ ��� ������

                                f_path_ok = true;
                                }
                            else
                                {
                                f_path_ok = false;
                                }
                            }
                        else
                            {
                            // ��� �������� ���������
                            f_path_ok = false;
                            }
                        */
                        }
                    else
                        {
                        // ������ json ������
                        f_path_ok = false;
                        }
                    }
                else
                    {
                    // ������ ��������� - ������������ � ��������� ������ ����� ��������� ���������
                    f_path_ok = false;
                    }
                break;
                }
            default:
                {
                rslt = false;
                f_path_ok = false;
                break;
                }
            }
        if (!rslt) break;
        rslt = pars_f1.p_parsekey_getNext (d_Name, d_type, d_indx);
        }
    rv = f_path_ok;
    if (rv) CurentPath = strkey;
    }
else
    {
    p_GetObj_size (ljson_data, '{', BaseJSN_TLP);   // ������������� ������� ����� �� �������� ������� ������
    //strkey = "";
    rv = true;
    }
return rv;
}



/*
bool TJSON::OpenPath (String strkey)
{
bool rv = false;
if (strkey != "")
    {
    String d_Name = "";
    EJSKEYTYPE d_type;
    long d_indx = 0; bool f_path_ok = false;
    p_GetObj_size (ljson_data, '{', BaseJSN_TLP);   // ������������� ��������� ����� �� ����� ������� json
    bool rslt = pars_f1.p_parsekey_getFist (strkey, d_Name, d_type, d_indx);
    while (rslt)
        {
        switch (d_type)
            {
            case EJSKEY_OBJ:
                {
                TLPObj outTlp;
                EJSNTYPE d_type_a;
                long indx = FindItemParam_fromOBJ (0, d_Name, outTlp, &d_type_a);
                if (indx >= 0 && outTlp.IsCorrect())
                    {
                    if (d_type_a == EJST_OBJ || d_type_a == EJST_ARRAY)
                        {
                        // � ������ ���������� ������� ������������� ����� ��������� �����
                        outTlp.f_is_array = false;
                        rslt = SetPathKey (outTlp);
                        f_path_ok = true;
                        }
                    else
                        {
                        f_path_ok = false;
                        }
                    }
                else
                    {
                    f_path_ok = false;
                    }
                break;
                }
            case EJSKEY_ARR:
                {
                TLPObj outTlp;
                EJSNTYPE d_type_a;
                long indx = FindItemParam_fromOBJ (0, d_Name, outTlp, &d_type_a);
                if (indx >= 0 && (d_type_a == EJST_ARRAY) && outTlp.IsCorrect())  // indx >= 0 && d_type_a == EJST_ARRAY && outTlp.IsCorrect() || d_type_a == EJST_OBJ
                    {
                    //SetPathKey (outTlp);        // ������� 11.05.21
                    TLPObj tlp = Item_get (&outTlp, d_indx);
                    if (tlp.IsCorrect())
                        {
                        // � ������ ���������� ������� ������������� ����� ��������� �����
                        tlp.f_is_array = true;
                        rslt = SetPathKey (tlp);
                        f_path_ok = true;
                        }
                    else
                        {
                        f_path_ok = false;
                        }
                    }
                else
                    {
                    f_path_ok = false;
                    }
                break;
                }
            default:
                {
                rslt = false;
                f_path_ok = false;
                break;
                }
            }
        if (!rslt) break;
        rslt = pars_f1.p_parsekey_getNext (d_Name, d_type, d_indx);
        }
    rv = f_path_ok;
    }
else
    {
    p_GetObj_size (ljson_data, '{', BaseJSN_TLP);   // ������������� ������� ����� �� �������� ������� ������
    rv = true;
    }
CurentPath = strkey;
return rv;
}

*/


// ������� ���������� ������� ������� ��� ������� ��� ����������� ��������
// ���������� ������ ������ ������� ��� �������, ���������� ����� ��������
long TJSON::p_GetObj_size (char *lCurRam, char objarr_symb, TLPObj &outTlp)       //  char **lstrt, char **lstpt
{
long rv = 0;
if (lCurRam)
    {
    char *lStartRam = 0;
    long o_cnt = 0;
    long a_cnt = 0;
    char dat;
    while (true)
        {
        dat = lCurRam[0];
        if (!dat)
            {

            break;
            }
        else
            {
            if (dat == '[')
                {
                if (objarr_symb == '[' && !a_cnt && !lStartRam) lStartRam = lCurRam;
                a_cnt++;
                }
            if (dat == '{')
                {
                if (objarr_symb == '{' && !o_cnt && !lStartRam) lStartRam = lCurRam;
                o_cnt++;
                }
            if (dat == ']')
                {
                if (a_cnt)
                    {
                    a_cnt--;
                    if (!a_cnt)
                        {
                        if (objarr_symb == '[')
                            {
                            //
                            if (o_cnt == 0 && lStartRam) rv = (lCurRam - lStartRam) + 1;
                            }
                        }
                    }
                else
                    {
                    // error
                    }
                }
            if (dat == '}')
                {
                if (o_cnt)
                    {
                    o_cnt--;
                    if (!o_cnt)
                        {
                        if (objarr_symb == '{')
                            {
                            if (a_cnt == 0 && lStartRam) rv = (lCurRam - lStartRam) + 1;
                            }
                        }
                    }
                else
                    {
                    // error
                    }
                }
            if (rv)
                {
                outTlp.lStart = lStartRam;
                outTlp.lStop = lCurRam;
                if (objarr_symb == '[')
                    {
                    outTlp.f_is_array = true;
                    }
                else
                    {
                    outTlp.f_is_array = false;
                    }
                break;
                }
            }
        lCurRam++;
        }
    }
return rv;
}



bool TJSON::SetPathKey (TLPObj const &outval)
{
bool rv = false;
if (outval.IsCorrect())
    {
    BaseJSN_TLP = outval;
    rv = true;
    }
return rv;
}



long TJSON::Item_AddRawField (TDString const &jsn_rawlineitem)
{
long rv = -1;
JSONARRITEMPAR par;
GetItem_RamPos_inx (0, 0xFFFFFF, par);
long amount = par.item_amount;
if (amount)
    {
    if (GetItem_RamPos_inx (0, (amount - 1), par))
        {
        if (par.end_ofs >=0)
            {
            //JSON_data.insert ((par.global_ofs + par.end_ofs + 1), ("," + jsn_rawlineitem).c_str());
            JSON_data.insert ((par.global_ofs + par.end_ofs), ("," + jsn_rawlineitem).c_str());
            rv = amount;
            }
        }
    }
else
    {
    if (par.m_enter_ofs != -1)
        {
        //JSON_data.insert ((par.global_ofs + par.m_enter_ofs + 2), jsn_rawlineitem.c_str());
        JSON_data.insert ((par.global_ofs + par.m_enter_ofs + 1), jsn_rawlineitem.c_str());
        rv = 0;
        }
    }

if (rv >= 0) UpdateLP ();
return rv;
}



bool TJSON::Item_insert (TDString const &JSON_insert, long i_array)
{
bool rv = false, rsltf;
JSONARRITEMPAR par;
long el_amount;
if (i_array >= 0)
    {
    rsltf = GetItem_RamPos_inx (0, i_array, par);
    if (rsltf)
        {
        if (par.stop_ofs != -1)
            {
            if (par.stop_ofs >= par.start_ofs)
                {
                TDString sddstr = "";
                JSON_data.insert ((par.global_ofs + par.start_ofs + 1), (JSON_insert + ',').c_str());
                rv = true;
                }
            }
        }
    }
if (rv) UpdateLP ();
return rv;
}






bool TJSON::Item_delete (long i_array)
{
bool rv = false, rsltf;
JSONARRITEMPAR par;
long el_amount;
if (i_array >= 0)
    {
    unsigned long start_ofs = 0, inc_v = 0;
    rsltf = GetItem_RamPos_inx (0, i_array, par);
    if (rsltf)
        {
        if (par.prevcoma_ofs != -1)
            {
            start_ofs = par.prevcoma_ofs;
            }
        else
            {
            start_ofs = par.start_ofs;
            inc_v = 1;
            }
        if (par.stop_ofs != -1)
            {
            if (par.stop_ofs >= start_ofs)
                {
                unsigned long sz = 0;
                if (par.coma_ofs != - 1) sz = par.coma_ofs - start_ofs + inc_v;
                if (par.end_ofs != - 1) sz = par.end_ofs - start_ofs;
                if (sz)
                    {
                    JSON_data.erase ((par.global_ofs + start_ofs + 1), sz);
                    rv = true;
                    }
                }
            }
        }
    }
if (rv) UpdateLP ();
return rv;
}




bool TJSON::Item_Get (long i_array, TDString &itemfield)
{
bool rv = false;
JSONARRITEMPAR par;
TLPObj IN_Tlp;
long el_amount;
if (i_array >= 0)
    {
    IN_Tlp = Item_get (0, i_array);
    IN_Tlp.TwoCuteTxt ();
    if (IN_Tlp.IsCorrect())
        {
        itemfield = TLP_to_Str (IN_Tlp);
        rv = true;
        }
    }
return rv;
}




bool TJSON::Item_replace (TDString const &JSON_replace, long i_array)
{
// String &JSON_inp
bool rv = false;
JSONARRITEMPAR par;
long el_amount;
if (i_array >= 0)
    {
    if (GetItem_RamPos_inx (0, i_array, par))
        {
        if (Item_insert (JSON_replace, i_array))
            {
            if (Item_delete ((i_array + 1)))
                {
                rv = true;
                }
            }
        }
    }
return rv;
}




bool TJSON::Add_Obj_new (TDString const &obj_name)
{
bool rv = false;
if (!Item_NameCheck (obj_name))
    {
    TDString new_item = "\"" + obj_name + "\":" + "{}";
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}


bool TJSON::Add_Obj (TDString const &obj_name, TDString const &obj_data)
{
bool rv = false;
if (!Item_NameCheck (obj_name))
    {
    TDString new_item = "\"" + obj_name + "\":" + obj_data;
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}



bool TJSON::Add_Arr_new (TDString const &arr_name)
{
bool rv = false;
if (!Item_NameCheck (arr_name))
    {
    TDString new_item = "\"" + arr_name + "\":" + "[]";
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}


bool TJSON::Add_Arr (TDString const &arr_name, TDString const &arr_data)
{
bool rv = false;
if (!Item_NameCheck (arr_name))
    {
    TDString new_item = "\"" + arr_name + "\":" + arr_data;
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}



bool TJSON::Add_Str (TDString const &str_name, TDString const &str_val)
{
bool rv = false;
if (!Item_NameCheck (str_name))
    {
    // ������������� ����� ������� �� ������
    TDString val = str_val;
    shield_set_to (val, true);
    TDString new_item = "\"" + str_name + "\":" + "\"" + val + "\"";
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}



bool TJSON::Add_Bool (TDString const &str_name, bool const &bool_val)
{
bool rv = false;
if (!Item_NameCheck (str_name))
    {
    TDString param = "false";
    if (bool_val) param = "true";
    TDString new_item = "\"" + str_name + "\":" + param;
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}




bool TJSON::Add_Numb (TDString const &str_name, long const &val)
{
bool rv = false;
if (!Item_NameCheck (str_name))
    {
    TDString new_item = "\"" + str_name + "\":" + LongToStr(val);
    if (Item_AddRawField (new_item) >= 0) rv = true;
    }
return rv;
}


// ������ ��������, �������� ��� �� �����������
bool TJSON::ChangeParamTo_str (TDString const &str_name, TDString const &new_param)
{
bool rv = false;
TLPObj pos_param;
long indx = FindItemParam_fromOBJ (0, str_name, pos_param, 0);
if (indx >= 0)
    {
    TDString newchange_name = "\""+str_name+"\":"+ "\"" + new_param + "\"";
    rv = Item_replace (newchange_name, indx);
    }
return rv;
}



bool TJSON::ChangeParamTo_long (TDString const &str_name, long new_param)
{
bool rv = false;
TLPObj pos_param;
long indx = FindItemParam_fromOBJ (0, str_name, pos_param, 0);
if (indx >= 0)
    {
    TDString newchange_name = "\""+str_name+"\":"+ LongToStr(new_param);
    rv = Item_replace (newchange_name, indx);
    }
return rv;
}



bool TJSON::ChangeParamTo_bool (TDString const &str_name, bool new_param)
{
bool rv = false;
TLPObj pos_param;
long indx = FindItemParam_fromOBJ (0, str_name, pos_param, 0);
if (indx >= 0)
    {
    TDString val = "false";
    if (new_param) val = "true";
    TDString newchange_name = "\""+str_name+"\":"+ val;
    rv = Item_replace (newchange_name, indx);
    }
return rv;
}



bool TJSON::Item_NameCheck (TDString const &str_name)
{
bool rv = false;
TLPObj pos_param;
if (FindItemParam_fromOBJ (0, str_name, pos_param, 0) >= 0) rv = true;
return rv;
}


