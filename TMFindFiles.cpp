#include "TMFindFiles.h"
#include "filerut.hpp"
#include "textrut.hpp"
#include "windows.h"


TMFindFiles::TMFindFiles ()
{
BaseSearchName = "";
HNDLSearchRaw = INVALID_HANDLE_VALUE;
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
    BaseSearchName = strpathe;
    }
else
    {
    BaseSearchName = "";
    }
return rv;
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
        TDString fullnames = BaseSearchName + FndFile;
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
TDString FullFiltr = BaseSearchName + "*.*";
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
if (BaseSearchName != "")
    {
    TDString strpathe = BaseSearchName + filterstr;
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
if (BaseSearchName != "")
    {
    TDString strpathe = BaseSearchName + filterstr;
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





 