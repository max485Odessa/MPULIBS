#ifndef _H_STM_JSON_H_
#define _H_STM_JSON_H_

#ifdef ISWINDOWS
    #include "TMWriteStream.hpp"
    #include "TMReadStream.hpp"
#endif

#include "STMSTRING.H"


enum EJSNTYPE {EJST_NONE = 0, EJST_DIG = 1, EJST_STRING = 2, EJST_OBJ = 3, EJST_ARRAY = 4};
//enum EJSKEYTYPE {EJSKEY_VALUE = 0, EJSKEY_OBJ = 1, EJSKEY_ARR = 2};
enum EJSKEYTYPE {EJSKEY_SINGLE = 0, EJSKEY_INDEXES = 1};
enum EJSEXTYPE {EJSEXTYPE_NONE, EJSEXTYPE_DIG_BOOL, EJSEXTYPE_DIG_LONG32, EJSEXTYPE_DIG_LONG64, EJSEXTYPE_DIG_FLOAT, EJSEXTYPE_STRING, EJSEXTYPE_OBJ, EJSEXTYPE_ARRAY, EJSEXTYPE_ENDENUM = 7};

/*
пример работы с массивами:
OpenPath ("path.path.array") -  Открытие области видимости на массив посредством  указывается без индексных скобок[].
OpenPath ("array[]") -  Открытие области видимости на элемент массива без указания индекса элемента :
    приравнимается к открытию элемента массива с индесом = 0; OpenPath ("array[0]").
OpenPath ("array[0]") - в случае когда типом элемента массива есть обьект область видимости устанавливается на него
*/



typedef struct {
    long global_ofs;    // глобальное смещение от начала
    long m_enter_ofs;   // первые кавычки
    long prevcoma_ofs;  // расположение предыдущей запятой
    long start_ofs;     // обнаружение первого символа элемента массива
    long stop_ofs;      // обнаружение завершающего символа элемента массива
    long coma_ofs;      // обнаружение запятой
    long end_ofs;       // обнаружение символа конца данных ( массива или обьекта )
    long item_amount;   // количество элементов в массиве
} JSONARRITEMPAR;


#define C_JSONSTRINGITEMSIZE_MAX 2048


class TLPObj {
    private:
    public:
        TLPObj ();
        void Start (char *adr);
        bool IsCorrect ();
        void TwoCuteTxt ();     // отрезает ненужные символы с начала и с конца
        void Clear();
        long Size ();

        char *lStart;
        char *lStop;
        bool f_is_array;
};


#define C_PARSEKEYSIZE_MAX 2048
#define C_JSONFPNAMESIZE_MAX 1024   // максимальная размер имени параметра

class TParsJSNKey {
    private:
        char bufrslt[C_PARSEKEYSIZE_MAX];
        TSTMSTRING Str_rslt;
        
        char bufsindx[64];
        TSTMSTRING str_indx;

        char bufskf[C_PARSEKEYSIZE_MAX];
        TSTMSTRING jsSeqKey_find;
        char *jslSqKey;
        long jsIndx;

    public:
      // функции и переменные по разбору path_key
      TParsJSNKey();
      bool p_parsekey_getFist (const char *lp_key, TSTMSTRING &d_Name, EJSKEYTYPE &d_type, long &d_indx);  // ++   TSTMSTRING const &JS_key
      bool p_parsekey_getNext (TSTMSTRING &d_Name, EJSKEYTYPE &d_type, long &d_indx);            // ++
};


class STMJSON {
  private:
	#ifdef ISWINDOWS
	TDString JsonFileDirPath;
	#endif


    TParsJSNKey pars_f1;    // ++
    
    void Clear ();  // ++

    TLPObj BaseJSN_TLP;
    long p_GetObj_size (char *lCurRam, char objarr_symb, TLPObj &outTlp);       // ++
    bool GetItem_RamPos_inx (TLPObj *IN_Tlp, long i_array, JSONARRITEMPAR &lpJS_par);       // ++
    TLPObj Item_get (TLPObj *IN_Tlp, long i_array);  // ++

    // делает экстракт выбранной рамками записи (имя, параметр, тип параметра)
    // если область поиска не указана - выбирается текущие рамки
    EJSNTYPE ExtractTypeParam (TLPObj *IN_Tlp, TSTMSTRING &outname, TLPObj &outval);  // ++

    long FindItemParam_fromOBJ (TLPObj *IN_Tlp, const char *nameparam, TLPObj &outParam, EJSNTYPE *d_type);   // ++ находит поле по имени и возвращает его тип и рамки размещения
    void TLP_to_Str (TLPObj &tlp, TSTMSTRING *dst_str);      // преобразует данные по указателям в STRING         ++
    bool SetPathKey (TLPObj &outval);     // устанавливает физическое ограничение по поиску элементнов в JSON структуре

    EJSEXTYPE GetTypeString (const char *name);               // определяет тип данных по строке
    //void shield_set_to (TSTMSTRING &str, bool f_on);        // установить/снять екранировку строки

    void CreateStringFromLine_cnt (TSTMSTRING &OutStr, char *lpAdr,unsigned long CntBt);

    bool f_load_file;
    char *loadfile_alloc;

  public:
    //TSTMSTRING JSON_data;

    char *ljson_data;
    void ResetLP ();

  public:
  
    STMJSON ();
    ~STMJSON();
    #ifdef ISWINDOWS
        bool Load_JSON (char *flname);
    #endif

    void SetJSON_data (char *lstr);

    long GetCurObj_size ();
    long GetCurObj_ItemAmount ();           // проверяет целостность формата текущего обьекта и выводит количество Items (текущего обьекта или массива)
    bool OpenPath (const char *lpath);			// const TSTMSTRING &strkey устанавливает PATHKEY к обьекту (не к элементу обьекта)

    bool GetItem (const char *ItmemName, TSTMSTRING *d_ItemVal, EJSNTYPE *d_type);		// возвращает значение по имени (массив, обьект или значение) и тип обьекта
    bool GetItemExtType (TSTMSTRING &ItmemName, TSTMSTRING &d_ItemVal, EJSEXTYPE *d_type);		// возвращает значение по имени (массив, обьект или значение) и тип обьекта расширенный
    long Item_Find (const char *jsn_rawlineitem);
    bool Item_NameCheck (const char *str_name); // ++
    bool Item_Get (long i_array, TSTMSTRING *itemfield);    // извлекает item, для извлечения array elements



    // извлекает значение параметра по имени в открытом пути
    bool Get_Str ( const char *name, TSTMSTRING *lpvalue);
    bool Get_Bool (const char *name, bool *lpvalue);
    bool Get_Numb (const char *name, long  *lpvalue);
    bool Get_Numb_F (const char *name, float  *lpvalue);
    bool Get_Arr (const char *name, TSTMSTRING  *lpvalue);
    bool Get_Obj (const char *name, TSTMSTRING  *lpvalue);

    void CuteCommaStr (TSTMSTRING &JSON_strparam);

};


#endif


 