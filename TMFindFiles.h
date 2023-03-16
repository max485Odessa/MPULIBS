#ifndef _HH_TMFINDFILES_HH_
#define _HH_TMFINDFILES_HH_

#include "windows.h"
#include "vcl.h"
#include "textrut.hpp"
#include "filerut.hpp"

class TMFindFiles{
    private:
        TDString BaseSearchName;
        WIN32_FIND_DATA FDRaw;
        HANDLE HNDLSearchRaw;
    protected:
        TDString GetCurentFileName();
        void CloseSearch ();
        bool FindFistRaw (TDString &OutFileName);
        bool FindNextRaw (TDString &OutFileName);
    public:
        TMFindFiles ();
        char SetSearchPath (TDString strpathe);
        unsigned long DeleteAllFiles (TDString filterExt);
        static bool ReMove_File (TDString const &cur_file_name, TDString const &new_file_nm);
        bool FindFist_File (TDString &OutFileName);
        bool FindNext_File (TDString &OutFileName);
        bool FindFist_Dir (TDString &OutFileName);
        bool FindNext_Dir (TDString &OutFileName);
        unsigned long GetListFileName (TDString filterstr, TDString &DestList, unsigned long CntFiles);
        unsigned long GetListDirName (TDString filterstr, TDString &DestList, unsigned long CntFiles);
};


#endif



 