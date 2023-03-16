#ifndef _H_MEMOBJ_H_
#define _H_MEMOBJ_H_

#include <vcl.h>
#include "TMReadStream.hpp"
#include "TMWriteStream.hpp"
#include "textrut.hpp"


using namespace TEX;


class TMEMOBJ {
        HLOCAL HlpMem;
        void *lpRam;
        unsigned long CurrentSize;
        unsigned long MaxAllocSize;
        bool InternalAlloc (unsigned long sszs);    // выделяет память из заранее выделенного резерва или с помощью системных функций
        bool AllocMem (unsigned long sizes);
        bool AddToBack (unsigned long sizes);
        void Init ();
        bool Del ();
        TMCreateReadStream Rd1;
        TMCreateWrireStream Wr1;

    public:
        TMEMOBJ ();
        ~TMEMOBJ ();
        bool LoadFile (char *lpFileName);
        bool SaveFile (char *lpFileName);
        unsigned long GetMemSize ();
        bool InsertTo (unsigned long offset, unsigned long sizes);
        bool DelFromBack (unsigned long sizes);
        bool DelFrom (unsigned long offset, unsigned long sizes);
        bool WriteMem (BUFPAR *lpSrc, unsigned long DestOfs);
        bool ReadMem (BUFPAR *lpDst, unsigned long SrcOfs);
        void *GetBaseAdress ();
        bool CreateString (TDString &str_out);

};

#endif


 