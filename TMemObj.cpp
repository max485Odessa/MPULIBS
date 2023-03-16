#include <vcl.h>
#include "tmemobj.hpp"
#include "textrut.hpp"
#include "TMReadStream.hpp"
#include "TMWriteStream.hpp"
#include "windows.h"


TMEMOBJ::TMEMOBJ ()
{
Init ();
}



void TMEMOBJ::Init ()
{
HlpMem = 0;
lpRam = 0;
CurrentSize = 0;
MaxAllocSize = 0;
}


TMEMOBJ::~TMEMOBJ ()
{
Del ();
}




void *TMEMOBJ::GetBaseAdress ()
{
return lpRam;
}


bool TMEMOBJ::InternalAlloc (unsigned long sszs)
{
bool rv = false;
unsigned long reservdelt = MaxAllocSize - CurrentSize;
if (reservdelt >= sszs)
    {
    CurrentSize = CurrentSize + sszs;
    rv = true;
    }
else
    {
    unsigned long ndalc = sszs - reservdelt;
    rv = AddToBack (ndalc);
    }
return rv;
}



// если память небыла выделена до этого, довыделяет память до нужного размера
bool TMEMOBJ::WriteMem (BUFPAR *lpSrc, unsigned long DestOfs)
{
bool rv = false;
if (lpSrc)      // выделить память если надо и скопировать туда данные если указан указатель источника
    {
    if (lpSrc->sizes)
        {
        bool reslt = true;
        unsigned long szllc = 0;
        if (DestOfs >= CurrentSize)
            {
            szllc = DestOfs - CurrentSize + lpSrc->sizes;   // выделяемый размер, если указатель на память больше физического размера
            }
        else
            {
            szllc = CurrentSize - DestOfs;
            if (szllc <= lpSrc->sizes)
                {
                szllc = lpSrc->sizes - szllc;
                }
            else
                {
                szllc = 0;
                }
            }
        if (szllc) reslt = InternalAlloc (szllc);   // выделить память если необходимо
        if (reslt)
            {
            if (lpSrc->lpRam && lpRam) CopyMemorySDC ((char*)lpSrc->lpRam, (char*)lpRam + DestOfs, lpSrc->sizes);
            rv = true;
            }
        }
    else
        {
        rv = true;
        }
    }
return rv;
}




bool TMEMOBJ::ReadMem (BUFPAR *lpDst, unsigned long SrcOfs)
{
bool rv = false;
if (lpDst && lpRam)
    {
    if (lpDst->sizes)
        {
        if (lpDst->lpRam)
            {
            unsigned long RdSize = 0;
            unsigned long FillSize = 0;
            char *lFillAdr = (char*)lpDst->lpRam;
            if (SrcOfs >= CurrentSize)
                {
                FillSize = lpDst->sizes;
                }
            else
                {
                unsigned long PreMemSz = CurrentSize - SrcOfs;
                if (PreMemSz >= lpDst->sizes)
                    {
                    RdSize = lpDst->sizes;
                    }
                else
                    {
                    RdSize = PreMemSz;
                    FillSize = lpDst->sizes - RdSize;
                    }
                if (FillSize) lFillAdr = lFillAdr + RdSize;
                }
            if (RdSize) lFillAdr = CopyMemorySDC ((char*)lpRam + SrcOfs, (char*)lpDst->lpRam, RdSize);
            if (FillSize) FillMems (lFillAdr, 0, FillSize);
            rv = true;
            }
        }
    else
        {
        rv = true;
        }
    }
return rv;
}



unsigned long TMEMOBJ::GetMemSize ()
{
return CurrentSize;
}




bool TMEMOBJ::LoadFile (char *lpFileName)
{
bool rv = false;
if (Rd1.OpenStream (lpFileName))
    {
    unsigned long Flen = Rd1.GetFileSize();
    if (Flen)
        {
        if (AllocMem (Flen))
            {
            unsigned long rdlen = Rd1.ReadData((char*)lpRam, Flen);
            if (rdlen == Flen) rv = true;
            }
        }
    Rd1.CloseStream();
    }
return rv;
}



bool TMEMOBJ::SaveFile (char *lpFileName)
{
bool rv = false;
if (lpRam)
    {
    if (Wr1.CreateStream(lpFileName))
        {
        if (CurrentSize)
            {
            Wr1.WriteBlock((char*)lpRam, CurrentSize);
            }
        rv = true;
        Wr1.CloseStream();
        }
    }
return rv;
}



bool TMEMOBJ::AllocMem (unsigned long sizes)
{
bool rv = false;
if (sizes)
    {
    if (!CurrentSize)
        {
        HlpMem = ::LocalAlloc (LMEM_MOVEABLE,sizes);
        if (HlpMem)
            {
            lpRam = ::LocalLock (HlpMem);
            if (lpRam)
                {
                CurrentSize = sizes;
                MaxAllocSize = sizes;
                rv = true;
                }
            }
        }
    else
        {
        if (sizes > CurrentSize)
            {
            rv = AddToBack ((sizes - CurrentSize));
            }
        else
            {
            if (sizes < CurrentSize)
                {
                rv = DelFromBack ((CurrentSize - sizes));
                }
            else
                {
                rv = true;
                }
            }
        }
    }
return rv;
}






bool TMEMOBJ::Del ()
{
bool rv = false;
if (CurrentSize && lpRam)
    {
    if (LocalFree(HlpMem) == NULL) rv = true;
    Init ();
    }
return rv;
}




bool TMEMOBJ::AddToBack (unsigned long sizes)
{
bool rv = false;
if (lpRam)
    {
    bool rslt = LocalUnlock(HlpMem);
    if (!rslt)
        {
        if (GetLastError () == NO_ERROR) rslt = true;
        }
    if (rslt)
        {
        CurrentSize = CurrentSize + sizes;
        HlpMem = ::LocalReAlloc (HlpMem, CurrentSize, LMEM_MOVEABLE);
        if (HlpMem)
            {
            lpRam = ::LocalLock(HlpMem);
            if (lpRam)
                {
                if (CurrentSize > MaxAllocSize) MaxAllocSize = CurrentSize;
                rv = true;
                }
            }
        }
    }
else
    {
    rv = AllocMem (sizes);
    }
return rv;
}




bool TMEMOBJ::InsertTo (unsigned long offsetTo, unsigned long sizes)
{
bool rv = false;
if (lpRam)
    {
    if (offsetTo >= CurrentSize)
        {
        // сдвигать не надо, просто выделить
        unsigned long ndalloc = offsetTo - CurrentSize + sizes;
        rv = AddToBack (ndalloc);
        }
    else
        {
        unsigned long movesizes = CurrentSize - offsetTo;
        if (AddToBack (sizes))
            {
            char *lpSrc = ((char*)lpRam) + offsetTo;
            MoveDownMemory (lpSrc, sizes, movesizes);
            rv = true;
            }
        }
    }
return rv;
}




bool TMEMOBJ::DelFromBack (unsigned long sizes)
{
bool rv = false;
if (lpRam)
    {
    if (CurrentSize >= sizes)
        {
        CurrentSize = CurrentSize - sizes;
        }
    else
        {
        CurrentSize = 0;
        }
    rv = true;
    }
return rv;
}



bool TMEMOBJ::DelFrom (unsigned long offset, unsigned long sizes)
{
bool rv = false;
if (lpRam && sizes)
    {
    if (offset < CurrentSize)
        {
        unsigned long maxbacksize = CurrentSize - offset;
        if (sizes >= maxbacksize)
            {
            sizes = maxbacksize;
            CurrentSize = CurrentSize - sizes;
            }
        else
            {
            char *lpDest = (char *)lpRam;
            lpDest = lpDest + offset;
            char *lpSrc = lpDest + sizes;
            unsigned long movesize = maxbacksize - sizes;
            CopyMemorySDC (lpSrc, lpDest, movesize);
            CurrentSize = CurrentSize - sizes;
            }
        rv = true;
        }
    else
        {
        rv = true;
        }
    }
return rv;
}



bool TMEMOBJ::CreateString (TDString &str_out)
{
bool rv = true;
if (CurrentSize)
    {
    CreateStringFromLine_cnt (str_out, (char*)lpRam, CurrentSize);
    }
else
    {
    str_out = "";
    }
return rv;
}



