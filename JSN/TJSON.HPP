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


class TParsJSNKey {
    private:
        TDString jsSeqKey_find; char *jslSqKey; long jsIndx;
    public:
      // функции и переменные по разбору path_key
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

    // делает экстракт выбранной рамками записи (имя, параметр, тип параметра)
    // если область поиска не указана - выбирается текущие рамки
    EJSNTYPE ExtractTypeParam (TLPObj *IN_Tlp, TDString &outname, TLPObj &outval);  // ++

    long FindItemParam_fromOBJ (TLPObj *IN_Tlp, TDString const &nameparam, TLPObj &outParam, EJSNTYPE *d_type);   // ++ находит поле по имени и возвращает его тип и рамки размещения
    TDString TLP_to_Str (TLPObj const &tlp);      // преобразует данные по указателям в STRING         ++
    bool SetPathKey (TLPObj const &outval);     // устанавливает физическое ограничение по поиску элементнов в JSON структуре

    EJSEXTYPE GetTypeString (const TDString &name);               // определяет тип данных по строке
    void shield_set_to (TDString &str, bool f_on);        // установить/снять екранировку строки

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
    long GetCurObj_ItemAmount ();           // проверяет целостность формата текущего обьекта и выводит количество Items (текущего обьекта или массива)
    bool OpenPath (TDString strkey);			// устанавливает PATHKEY к обьекту (не к элементу обьекта и не к массиву елементов)


    bool GetItem (TDString const &ItmemName, TDString &d_ItemVal, EJSNTYPE *d_type);		// возвращает значение по имени (массив, обьект или значение) и тип обьекта
    bool GetItemExtType (TDString const &ItmemName, TDString &d_ItemVal, EJSEXTYPE *d_type);		// возвращает значение по имени (массив, обьект или значение) и тип обьекта расширенный
    long Item_AddRawField (TDString const &jsn_rawlineitem);  // ++  добавляет имя и параметр в конец обьекта
    long Item_Find (TDString const &jsn_rawlineitem);
    bool Item_insert (TDString const &JSON_insert, long i_array); // ++ добавляет имя и параметр в текущюю позицию
    bool Item_replace (TDString const &JSON_replace, long i_array);  // ++ заменяет
    bool Item_delete (long i_array);        // ++
    bool Item_NameCheck (TDString const &str_name); // ++
    bool Item_Get (long i_array, TDString &itemfield);    // извлекает item, для извлечения array elements

    bool Add_Obj (TDString const &obj_name, TDString const &obj_data);
    bool Add_Obj_new (TDString const &obj_name);  // ++
    bool Add_Arr (TDString const &arr_name, TDString const &arr_data);
    bool Add_Arr_new (TDString const &arr_name);   // ++
    bool Add_Str (TDString const &str_name, TDString const &str_val);    // ++
    bool Add_Numb (TDString const &str_name, long const &str_val);    // ++
    bool Add_Bool (TDString const &str_name, bool const &bool_val);

    // извлекает значение параметра по имени в открытом пути
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


