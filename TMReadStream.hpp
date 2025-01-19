#ifndef TMCrRead1
#define TMCrRead1

#include <vcl.h>
#include "filerut.hpp"
#include <stdint.h>
#include "STMSTRING.H"

class TMCreateReadStream{
public:
        TMCreateReadStream();
		~TMCreateReadStream();
        static TDString GetExecPath ();
        unsigned char OpenStream (char *filename);
        void CloseStream ();
        uint32_t GetFileSize ();
        unsigned char AddReadIndex (int index);
        unsigned char SetReadIndex (unsigned int index);
        uint32_t GetCurrentIndex ();
        uint32_t ReadData (void *lpAdr, uint32_t size);
        uint32_t GetString (char *lpString, unsigned int maxsizesb);
        bool GetNextString (TDString &Dststr);
        bool GetAllText (TDString &Dststr);
        unsigned char DecIndex ();
        uint32_t ReadDataToNewFile (char *lpFileName, unsigned int wrsize);
        uint32_t ReadDataToNewFileAndDecoding (char *lpFileName, char *lpPassworde, unsigned int wrsize);
        uint32_t ReadDataToNewFileAndDecodingPink (char *lpFileName, char *lpPassworde, unsigned int wrsize);
        uint32_t ReadDataToRamAndDecodingPink (char *lpOutRamAdr, char *lpInput, char *lpPassworde, unsigned int wrsize);
        uint32_t GetCurrentASCIIString (unsigned char *lpDest, unsigned int sizebufs);
        uint32_t GetSelASCIIString (unsigned char *lpDest, unsigned int sizebufs, unsigned int strIndx);
        uint32_t GetStrings (TDString &Dest);
        char CompareString (TDString names);
        unsigned char ReadByteBX ();
        unsigned short ReadWordBX ();
        unsigned int ReadDwordBX ();
        unsigned char lpBufer[32768];
        unsigned int GlobalFileSize;

private:

        void ReadNextDataBufer ();
        bool NextStr ();
        unsigned char FlagEndBX;
        unsigned int GlobalIndexBuferRead;
        unsigned int LocalIndexBuferRead;
        unsigned int BuferDataSize;
        HANDLE Handll;
};

#endif
