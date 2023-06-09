#ifndef c_TTJSONE__h
#define c_TTJSONE__h
#include <vcl.h>

#include "TMWriteStream.hpp"
#include "TMReadStream.hpp"
#include "textrut.hpp"

enum EJSNTYPE {EJST_NONE = 0, EJST_DIG = 1, EJST_STRING = 2, EJST_OBJ = 3, EJST_ARRAY = 4};
//enum EJSKEYTYPE {EJSKEY_VALUE = 0, EJSKEY_OBJ = 1, EJSKEY_ARR = 2};
enum EJSKEYTYPE {EJSKEY_SINGLE = 0, EJSKEY_INDEXES = 1};
enum EJSEXTYPE {EJSEXTYPE_NONE, EJSEXTYPE_DIG_BOOL, EJSEXTYPE_DIG_LONG32, EJSEXTYPE_DIG_LONG64, EJSEXTYPE_DIG_FLOAT, EJSEXTYPE_STRING, EJSEXTYPE_OBJ, EJSEXTYPE_ARRAY, EJSEXTYPE_ENDENUM = 7};

/*
������ ������ � ���������:
OpenPath ("path.path.array") -  �������� ������� ��������� �� ������ �����������  ����������� ��� ��������� ������[].
OpenPath ("array[]") -  �������� ������� ��������� �� ������� ������� ��� �������� ������� �������� :
    �������������� � �������� �������� ������� � ������� = 0; OpenPath ("array[0]").
OpenPath ("array[0]") - � ������ ����� ����� �������� ������� ���� ������ ������� ��������� ��������������� �� ����


*/



typedef struct {
    long global_ofs;    // ���������� �������� �� ������
    long m_enter_ofs;   // ������ �������
    long prevcoma_ofs;  // ������������ ���������� �������
    long start_ofs;     // ����������� ������� ������� �������� �������
    long stop_ofs;      // ����������� ������������ ������� �������� �������
    long coma_ofs;      // ����������� �������
    long end_ofs;       // ����������� ������� ����� ������ ( ������� ��� ������� )
    long item_amount;   // ���������� ��������� � �������
} JSONARRITEMPAR;



class TLPObj {
    private:
    public:
        TLPObj ();
        void Start (char *adr);
        bool IsCorrect ();
        void TwoCuteTxt ();     // �������� �������� ������� � ������ � � �����
        void Clear();
        long Size ();
        
        char *lStart;
        char *lStop;
        bool f_is_array;
};


class TParsJSNKey {
    private:
        TDString jsSeqKey_find; char *jslSqKey; long jsIndx;
    public:
      // ������� � ���������� �� ������� path_key
      TParsJSNKey();
      bool p_parsekey_getFist (TDString const &JS_key, TDString &d_Name, EJSKEYTYPE &d_type, long &d_indx);  // ++
      bool p_parsekey_getNext (TDString &d_Name, EJSKEYTYPE &d_type, long &d_indx);            // ++
};


class TJSON {
  private:
    TDString JsonFileDirPath;

    TParsJSNKey pars_f1;    // ++
    
    void Clear ();  // ++

    TLPObj BaseJSN_TLP;
    long p_GetObj_size (char *lCurRam, char objarr_symb, TLPObj &outTlp);       // ++
    bool GetItem_RamPos_inx (TLPObj *IN_Tlp, long i_array, JSONARRITEMPAR &lpJS_par);       // ++
    TLPObj Item_get (TLPObj *IN_Tlp, long i_array);  // ++

    // ������ �������� ��������� ������� ������ (���, ��������, ��� ���������)
    // ���� ������� ������ �� ������� - ���������� ������� �����
    EJSNTYPE ExtractTypeParam (TLPObj *IN_Tlp, TDString &outname, TLPObj &outval);  // ++

    long FindItemParam_fromOBJ (TLPObj *IN_Tlp, TDString const &nameparam, TLPObj &outParam, EJSNTYPE *d_type);   // ++ ������� ���� �� ����� � ���������� ��� ��� � ����� ����������
    TDString TLP_to_Str (TLPObj const &tlp);      // ����������� ������ �� ���������� � STRING         ++
    bool SetPathKey (TLPObj const &outval);     // ������������� ���������� ����������� �� ������ ���������� � JSON ���������

    EJSEXTYPE GetTypeString (const TDString &name);               // ���������� ��� ������ �� ������
    void shield_set_to (TDString &str, bool f_on);        // ����������/����� ����������� ������

  public:
    TDString JSON_data;
    TDString CurentPath;
    char *ljson_data;
    void UpdateLP ();
	
  public:
  
    TJSON ();
    ~TJSON();
    TDString Get_JSON ();
    bool Load_JSON (char *flname);
    bool Save_JSON (TDString flname);
    void SetJSON_data (TDString strr);
    long GetCurObj_size ();
    long GetCurObj_ItemAmount ();           // ��������� ����������� ������� �������� ������� � ������� ���������� Items (�������� ������� ��� �������)
    bool OpenPath (TDString strkey);			// ������������� PATHKEY � ������� (�� � �������� ������� � �� � ������� ���������)


    bool GetItem (TDString const &ItmemName, TDString &d_ItemVal, EJSNTYPE *d_type);		// ���������� �������� �� ����� (������, ������ ��� ��������) � ��� �������
    bool GetItemExtType (TDString const &ItmemName, TDString &d_ItemVal, EJSEXTYPE *d_type);		// ���������� �������� �� ����� (������, ������ ��� ��������) � ��� ������� �����������
    long Item_AddRawField (TDString const &jsn_rawlineitem);  // ++  ��������� ��� � �������� � ����� �������
    long Item_Find (TDString const &jsn_rawlineitem);
    bool Item_insert (TDString const &JSON_insert, long i_array); // ++ ��������� ��� � �������� � ������� �������
    bool Item_replace (TDString const &JSON_replace, long i_array);  // ++ ��������
    bool Item_delete (long i_array);        // ++
    bool Item_NameCheck (TDString const &str_name); // ++
    bool Item_Get (long i_array, TDString &itemfield);    // ��������� item, ��� ���������� array elements

    bool Add_Obj (TDString const &obj_name, TDString const &obj_data);
    bool Add_Obj_new (TDString const &obj_name);  // ++
    bool Add_Arr (TDString const &arr_name, TDString const &arr_data);
    bool Add_Arr_new (TDString const &arr_name);   // ++
    bool Add_Str (TDString const &str_name, TDString const &str_val);    // ++
    bool Add_Numb (TDString const &str_name, long const &str_val);    // ++
    bool Add_Bool (TDString const &str_name, bool const &bool_val);

    // ��������� �������� ��������� �� ����� � �������� ����
    bool Get_Str (TDString const &name, TDString *lpvalue);
    bool Get_Bool (TDString const &name, bool *lpvalue);
    bool Get_Numb (TDString const &name, long  *lpvalue);
    bool Get_Numb_F (TDString const &name, float  *lpvalue);
    bool Get_Numb_LL (TDString const &name, long long *lpvalue);
    bool Get_Arr (TDString const &name, TDString  *lpvalue);
    bool Get_Obj (TDString const &name, TDString  *lpvalue);

    bool ChangeParamTo_str (TDString const &str_name, TDString const &new_param);  // ++
    bool ChangeParamTo_long (TDString const &str_name, long new_param);  // ++
    bool ChangeParamTo_bool (TDString const &str_name, bool new_param);  // ++

    void CuteCommaStr (TDString &JSON_strparam);

};


#endif


