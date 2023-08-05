#ifndef _HH_TMFINDFILES_HH_
#define _HH_TMFINDFILES_HH_

#include "windows.h"
#include "vcl.h"
#include "filerut.hpp"
#include "textrut.hpp"

class TMFindFiles{
    private:
        TDString BaseSearch;
        WIN32_FIND_DATA FDRaw;
        HANDLE HNDLSearchRaw;
    protected:
        TDString GetCurentFileName();
        void CloseSearch ();
        bool FindFistRaw (TDString &OutFileName);
        bool FindNextRaw (TDString &OutFileName);
    public:
        TMFindFiles ();
        static bool CreateDirPaths (const TDString &strpathe, bool f_open_cpath); // создает серии папок и устанавливает крайнюю созданную как текущюю
        char SetSearchPath (TDString strpathe);
        unsigned long DeleteAllFiles (TDString filterExt);
        static bool ReMove_File (TDString const &cur_file_name, TDString const &new_file_nm);
        static void GetCurrentDirectory (TDString &dst_dname);
        static bool SetCurrentDir (const TDString &dst_dname);
        bool FindFist_File (TDString &OutFileName);
        bool FindNext_File (TDString &OutFileName);
        bool FindFist_Dir (TDString &OutFileName);
        bool FindNext_Dir (TDString &OutFileName);
        unsigned long GetListFileName (TDString filterstr, TDString &DestList, unsigned long CntFiles);
        unsigned long GetListDirName (TDString filterstr, TDString &DestList, unsigned long CntFiles);
};


#endif



 