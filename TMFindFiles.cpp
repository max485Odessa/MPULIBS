#include "TMFindFiles.h"
#include "filerut.hpp"
#include "textrut.hpp"
#include "windows.h"
#include "STMSTRING.h"


TMFindFiles::TMFindFiles ()
{
BaseSearch = "";
HNDLSearchRaw = INVALID_HANDLE_VALUE;
}



bool TMFindFiles::SetCurrentDir (const TDString &dst_dname)
{
return SetCurrentDirectory (dst_dname.c_str());
}



bool TMFindFiles::CreateDirPaths (const TDString &strpathe, bool f_open_cpath)
{
bool rv = false;
TDString OutDiskName = "", OutPathNames = "", OutFileName = "";
TDString curpath = "";
if (!f_open_cpath) GetCurrentDirectory (curpath);
if (FullExtractFilenamePath ((char*)strpathe.c_str(), OutDiskName, OutPathNames, OutFileName))
    {
    unsigned long cntr = 0, fnd_cnt;
    char *ltxt = (char*)OutPathNames.c_str();
    TSTMSTRING strin (ltxt, ltxt, GetLenS (ltxt) + 1);
    strin.getcomastring_indx (0, 0xFFFF, '\\', &cntr);
    long ix = 0, fnd_ix = 0;
    
    TDString resltpath = "";
    resltpath += OutDiskName;
    resltpath += ":\\";

    static char buf[256];
    TSTMSTRING fsp (buf, sizeof(buf));

    bool rslt = SetCurrentDir (resltpath);
    while (rslt && ix < cntr)
        {
        fsp = "";
        strin.getcomastring_indx (&fsp, fnd_ix, '\\', &fnd_cnt);
        if (!fsp.Length()) break;

        resltpath += fsp.c_str(); resltpath += "\\";
        if (!SetCurrentDir (resltpath))
            {
            // папки небыло, создаем папку
            if (!CreateDirectory_E (fsp.c_str())) break;
            }
        if (!SetCurrentDir (resltpath)) break;

        fnd_ix++;
        ix++;
        }
    if (!fsp.Length())
        {
        if (ix == (cntr - 1)) rv = true;
        }
    }
if (!f_open_cpath) SetCurrentDir (curpath);
return rv;
}



TDString TMFindFiles::GetCurentFileName ()
{
TDString rv = "";
CreateStringFromLine (rv, FDRaw.cFileName);
return rv;
}



char TMFindFiles::SetSearchPath (TDString strpathe)
{
char rv = true;
char *lpath = (char*)strpathe.c_str();
unsigned long szstr = GetLenStr (lpath);
if (szstr > 2)
    {
    if (lpath[(szstr - 1)] != '\\') strpathe = strpathe + '\\';
    BaseSearch = strpathe;
    }
else
    {
    BaseSearch = "";
    }
return rv;
}



void TMFindFiles::GetCurrentDirectory (TDString &dst_dname)
{
TDString str = GetCurrentDir().c_str();
dst_dname = str.c_str();
}



bool TMFindFiles::ReMove_File (TDString const &cur_file_name, TDString const &new_file_nm)
{
bool rv = false;
char *lSrc = (char*)cur_file_name.c_str();
BOOL val = CopyFile(lSrc,new_file_nm.c_str(), true);
if (val == 0)
    {
    TDString extt_name = "";
    TDString newww = "";
    ExtractFileNameExt (new_file_nm, extt_name);
    unsigned long sz = ExtractFileNameBase (new_file_nm, newww);
    if (sz)
        {
        newww = newww + "_" + GenerateTimeString ('_') + "." + extt_name;
        val = CopyFile(lSrc,newww.c_str(), true);
        if (val != 0)
            {
            rv = DeleteFile_E (lSrc);
            //rv = true;
            }
        }
    }
else
    {
    rv = DeleteFile_E (lSrc);
    }
return rv;
}



unsigned long TMFindFiles::DeleteAllFiles (TDString filterExt)
{
unsigned long rv = 0;
TDString FndFile;
bool rslt = FindFist_File (FndFile);
TDString curExt;
bool err = false;
while (rslt)
    {
    curExt = ExtractFileExt (FndFile.c_str()).c_str();
    if (curExt == filterExt || filterExt == "*")
        {
        // расширение совпало с фильтром - удалить файл
        TDString fullnames = BaseSearch + FndFile;
        if (DeleteFile_E ((char*)fullnames.c_str())) rv++;
        }
    rslt = FindNext_File (FndFile);
    }
CloseSearch ();
return rv;
}




bool TMFindFiles::FindFistRaw (TDString &OutFileName)
{
bool rv = false;
OutFileName = "";
TDString FullFiltr = BaseSearch + "*.*";
HNDLSearchRaw = FindFirstFile (FullFiltr.c_str (),&FDRaw);
if (HNDLSearchRaw != INVALID_HANDLE_VALUE)
    {
    TDString SinglFileName = GetCurentFileName ();
    if (SinglFileName == "." || SinglFileName == "..")
        {
        bool rslt = FindNextRaw (SinglFileName);
        while (rslt)
            {
            if (SinglFileName != "." && SinglFileName != "..")
                {
                OutFileName = SinglFileName;
                rv = true;
                break;
                }
            rslt = FindNextRaw (SinglFileName);
            }
        }
    else
        {
        OutFileName = SinglFileName;
        rv = true;
        }
    }
return rv;
}



bool TMFindFiles::FindNextRaw (TDString &OutFileName)
{
bool rv = false;
OutFileName = "";
if (HNDLSearchRaw != INVALID_HANDLE_VALUE)
    {
    bool rslt = true;
    TDString SinglFileName;
    while (rslt)
        {
        rslt = FindNextFile (HNDLSearchRaw, &FDRaw);
        if (!rslt)
            {
            CloseSearch ();
            break;
            }
        SinglFileName = GetCurentFileName ();
        if (SinglFileName != "." && SinglFileName != "..")
            {
            rv = true;
            OutFileName = SinglFileName;
            break;
            }
        }
    }
return rv;
}



void TMFindFiles::CloseSearch ()
{
FindClose (HNDLSearchRaw);
HNDLSearchRaw = INVALID_HANDLE_VALUE;
}




unsigned long TMFindFiles::GetListFileName (TDString filterstr, TDString &DestList, unsigned long MaxCntFiles)
{
unsigned long rv = 0;
DestList = "";
if (BaseSearch != "")
    {
    TDString strpathe = BaseSearch + filterstr;
    if (MaxCntFiles)
        {
        TDString locstr = "";
        bool rslt = FindFistRaw (locstr);
        while (rslt && MaxCntFiles)
            {
            if (!(FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) // locstr !="." && locstr !=".." &&
                {
                rv++;
                MaxCntFiles--;
                DestList = DestList + locstr;
                if (!MaxCntFiles)
                    {
                    CloseSearch ();
                    break;
                    }
                DestList = DestList + "\r\n";
                }
            rslt = FindNextRaw (locstr);
            }
        }
    }
return rv;
}



unsigned long TMFindFiles::GetListDirName (TDString filterstr, TDString &DestList, unsigned long MaxCntFiles)
{
unsigned long rv = 0;
DestList = "";
if (BaseSearch != "")
    {
    TDString strpathe = BaseSearch + filterstr;
    if (MaxCntFiles)
        {
        TDString locstr = "";
        bool rslt = FindFistRaw (locstr);
        while (rslt && MaxCntFiles)
            {
            if ((FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) // locstr !="." && locstr !=".." &&
                {
                rv++;
                MaxCntFiles--;
                DestList = DestList + locstr;
                if (!MaxCntFiles)
                    {
                    CloseSearch ();
                    break;
                    }
                DestList = DestList + "\r\n";
                }
            rslt = FindNextRaw (locstr);
            }
        }
    }
return rv;
}




bool TMFindFiles::FindFist_File (TDString &OutFileName)
{
bool rv = false, rslt = false;
OutFileName = "";
rslt = FindFistRaw (OutFileName);
while (rslt)
    {
    if (!(FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
        rv = true;
        break;
        }
    rslt = FindNextRaw (OutFileName);
    }
return rv;
}



bool TMFindFiles::FindNext_File (TDString &OutFileName)
{
bool rv = false, rslt = false;
OutFileName = "";
if (HNDLSearchRaw != INVALID_HANDLE_VALUE) rslt = true;
while (rslt)
    {
    rslt = FindNextRaw (OutFileName);
    if (rslt)
        {
        if (!(FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
            rv = true;
            break;
            }
        }
    }
return rv;
}



bool TMFindFiles::FindFist_Dir (TDString &OutFileName)
{
bool rv = false, rslt = false;
OutFileName = "";
rslt = FindFistRaw (OutFileName);
while (rslt)
    {
    if (FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
        rv = true;
        break;
        }
    rslt = FindNextRaw (OutFileName);
    }
return rv;
}



bool TMFindFiles::FindNext_Dir (TDString &OutFileName)
{
bool rv = false, rslt = false;
OutFileName = "";
if (HNDLSearchRaw != INVALID_HANDLE_VALUE) rslt = true;
while (rslt)
    {
    rslt = FindNextRaw (OutFileName);
    if (rslt)
        {
        if (FDRaw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
            rv = true;
            break;
            }
        }
    }
return rv;
}





 