#ifndef TMCrWrite1
#define TMCrWrite1

#include <vcl.h>
#include "textrut.hpp"

class TMCreateWrireStream{
public:
        TMCreateWrireStream();
        ~TMCreateWrireStream();
        bool CreateStream (TDString FileName);
        bool CreateStream (char *lpFileName);
        bool CreateStreamLen (char *lpFileName, unsigned long sizes);
        bool OpenStream (TDString FileName);
        bool OpenOrCreateStream (TDString &FileName);
        bool OpenOrCreateStream (char* FileName);
        static unsigned long CopyFile (char *lpInputFN, char *lpOutFN);
        uint32_t ReadData (void *lp,uint32_t size);
        bool SetOffsetBegin (uint32_t offsetlong);

        unsigned char WriteByte (unsigned char data);
        unsigned char WriteShort (unsigned short datas);
        unsigned char WriteLong (unsigned long datas);
        char WriteString (TDString mstr);
        char WriteASCIIZ (char *lpStr);
	    char WriteOnlyCharsetStringAddNM (char *lpmstr);
	    char WriteOnlyCharsetString (char *lpmstr);
        unsigned char WriteHexByte(unsigned char data);
        char WriteTextIniStringParam (char *lpNameVariable,char *lpParam);
        char WriteTextIniIntParam (char *lpNameVariable,int paramint);

        unsigned int WriteFileCoding (char *lpFileName,char *lpPassCode);
        unsigned int WriteFileCodingPink (char *lpFileName,char *lpPassCode);
        
		
        uint32_t Write (void *lpRam, uint32_t counterb);
        unsigned int WriteFileX (char *lpFileName);

        unsigned char CloseStream ();
        unsigned int GlobalFileSize;
        
private:
        unsigned char lpBufer[32768];
        unsigned int IndexDataBufer;
        unsigned char flagdata;
        HANDLE Handll;
        unsigned char WriteBufer();
};


#endif
