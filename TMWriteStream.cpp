#include "filerut.hpp"
#include "TMWriteStream.hpp"
#include "TMReadStream.hpp"






TMCreateWrireStream::TMCreateWrireStream()
{
flagdata=0;
Handll=INVALID_HANDLE_VALUE;
IndexDataBufer=0;
memset(lpBufer,0,sizeof(lpBufer));
GlobalFileSize=0;
}



unsigned int TMCreateWrireStream::WriteFileCodingPink (char *lpFileName,char *lpPassCode)
{
unsigned int rv=0;
if (lpFileName && lpPassCode)
	{
	unsigned char *TechPass=(unsigned char *)"Bruliante-Incorparates";
	unsigned char *lpCurTechPass=TechPass;
	unsigned char *lpCurPassdata=(unsigned char *)lpPassCode;
	unsigned int LenPassword=TSTMSTRING::lenstr(lpPassCode);
	TMCreateReadStream ReadStr;
	if (ReadStr.OpenStream(lpFileName))
		{
		int curfsize=ReadStr.GetFileSize();
		if (curfsize)
			{
            unsigned char curbcod=234,tmpvvv;
			unsigned char curbyted;
			unsigned char maskleft=1;
			unsigned char maskright=128;
			while (curfsize)
				{
				curbyted=ReadStr.ReadByteBX();
                tmpvvv=curbyted;
                curbyted=curbyted ^ curbcod;
                curbyted=curbyted ^ TechPass[(curbcod & 0x0F)];
                curbcod=tmpvvv;

				if (LenPassword)
					{
					curbyted=curbyted ^ lpCurPassdata[0];
					lpCurPassdata++;
					if (!lpCurPassdata[0]) lpCurPassdata=(unsigned char *)lpPassCode;
					}
				curbyted=curbyted ^ lpCurTechPass[0];
				lpCurTechPass++;
				if (!lpCurTechPass[0]) lpCurTechPass=TechPass;
				curbyted=curbyted ^ maskright;
				curbyted=curbyted ^ maskleft;
				
				maskright=maskright >> 1;
				if (!maskright) maskright=128;
				maskleft=maskleft <<1;
				if (!maskleft) maskleft=1;
				
				curbyted=curbyted ^ (unsigned char)rv;



				if (WriteByte(curbyted))
					{
					curfsize--;
					rv++;
					
					}
				else
					{
					break;
					}
				}
			}
		ReadStr.CloseStream();
		}
	}
return rv;
}



unsigned int TMCreateWrireStream::WriteFileCoding (char *lpFileName,char *lpPassCode)
{
unsigned int rv=0;
if (lpFileName && lpPassCode)
	{
	char *TechPass="Bruliante-Incorparates";
	char *lpCurTechPass=TechPass;
	char *lpCurPassdata=lpPassCode;
	//unsigned int LenTechPass=GetLenStr(lpCurTechPass);
	unsigned int LenPassword=TSTMSTRING::lenstr(lpPassCode);
	TMCreateReadStream ReadStr;
	if (ReadStr.OpenStream(lpFileName))
		{
		int curfsize=ReadStr.GetFileSize();
		if (curfsize)
			{
			unsigned char curbyted;
			unsigned char maskleft=1;
			unsigned char maskright=128;
			while (curfsize)
				{
				curbyted=ReadStr.ReadByteBX();



				if (LenPassword)
					{
					curbyted=curbyted ^ lpCurPassdata[0];
					lpCurPassdata++;
					if (!lpCurPassdata[0]) lpCurPassdata=lpPassCode;
					}
				curbyted=curbyted ^ lpCurTechPass[0];
				lpCurTechPass++;
				if (!lpCurTechPass[0]) lpCurTechPass=TechPass;
				curbyted=curbyted ^ maskright;
				curbyted=curbyted ^ maskleft;
				
				maskright=maskright >> 1;
				if (!maskright) maskright=128;
				maskleft=maskleft <<1;
				if (!maskleft) maskleft=1;
				
				curbyted=curbyted ^ (unsigned char)rv;



				if (WriteByte(curbyted))
					{
					curfsize--;
					rv++;
					
					}
				else
					{
					break;
					}
				}
			}
		ReadStr.CloseStream();
		}
	}
return rv;
}



char TMCreateWrireStream::WriteOnlyCharsetStringAddNM (char *lpmstr)
{
char rv=0;
if (WriteOnlyCharsetString (lpmstr))
	{
	if (WriteByte(13))
		{
		rv=WriteByte(10);
		}
	}
return rv;
}



char TMCreateWrireStream::WriteOnlyCharsetString (char *lpRam)
{
char rv=0;
if (lpRam)
	{
	unsigned char datas;
	while (1)
		{
		datas=lpRam[0];
		if (datas<32)
			{
			rv=1;
			break;
			}
		if (!WriteByte(datas)) break;
		lpRam++;
		}	
	}
return rv;
}



bool TMCreateWrireStream::SetOffsetBegin (uint32_t offsetlong)
{
WriteBufer();
SeekFile_E (Handll,offsetlong,FILE_BEGIN);
return true;
}



TMCreateWrireStream::~TMCreateWrireStream()
{
CloseStream();
}




bool TMCreateWrireStream::OpenStream (TDString FileName)
{
bool rv = false;
Handll=OpenFile_E((char*)FileName.c_str());
flagdata=0;
IndexDataBufer=0;
GlobalFileSize=0;
if (Handll!=INVALID_HANDLE_VALUE)
        {
        // �������� ��������� ������ � �����
        GlobalFileSize=SeekFile_E (Handll,0,FILE_END);
        rv = true;
        }
return rv;
}







bool TMCreateWrireStream::CreateStream (TDString Filename)
{
bool rv = false;
Handll = CreateFile_E ((char*)Filename.c_str());
flagdata=0;
IndexDataBufer=0;
GlobalFileSize=0;
if (Handll != INVALID_HANDLE_VALUE) rv = true;
return rv;
}









bool TMCreateWrireStream::CreateStream(char *lpFileName)
{
bool rv = false;
if (lpFileName)
	{
	Handll = CreateFile_E (lpFileName);
    if (Handll != INVALID_HANDLE_VALUE) rv = true;
	flagdata=0;
	IndexDataBufer=0;
	GlobalFileSize=0;
	}
return rv;
}



bool TMCreateWrireStream::OpenOrCreateStream (char* FileName)
{
bool rv = OpenStream (FileName);
if (!rv) rv = CreateStream (FileName);
return rv;
}



bool TMCreateWrireStream::OpenOrCreateStream (TDString &FileName)
{
return OpenOrCreateStream ((char*)FileName.c_str());
}



bool TMCreateWrireStream::CreateStreamLen (char *lpFileName, unsigned long sizes)
{
bool rv = false;
if (CreateStream (lpFileName))
    {
    while (sizes)
        {
        WriteByte (0);
        sizes--;
        }
    rv = true;
    }
return rv;
}




unsigned char TMCreateWrireStream::CloseStream()
{
if (Handll != INVALID_HANDLE_VALUE)
        {
        if (flagdata) WriteBufer();
        //FlushFileBuffers(Handll);   //15.05.17
        CloseFile_E (Handll);
        }
memset(lpBufer,0,sizeof(lpBufer));

flagdata=0;
Handll=INVALID_HANDLE_VALUE;
IndexDataBufer=0;
GlobalFileSize=0;
return 1;
}



unsigned char TMCreateWrireStream::WriteBufer()
{
if (flagdata)
        {
        WriteFile_E((char*)lpBufer,IndexDataBufer,Handll);
        IndexDataBufer=0;
        flagdata=0;
        }
return 0;
}



unsigned char TMCreateWrireStream::WriteByte(unsigned char datas)
{
if (IndexDataBufer<sizeof(lpBufer))
        {
        lpBufer[IndexDataBufer]=datas;
        IndexDataBufer++;
        flagdata=1;
        }
else
        {
        WriteBufer();
        lpBufer[IndexDataBufer]=datas;
        IndexDataBufer++;
        flagdata=1;
        }
GlobalFileSize++;
return 1;
}



uint32_t TMCreateWrireStream::Write (void *lp, uint32_t counterb)
{
uint32_t rv = 0;
uint8_t *lpRam = (uint8_t*)lp;
if (lpRam && counterb)
        {
        while (counterb)
                {
                if (WriteByte(lpRam[0]))
                        {
                        lpRam++;
                        counterb--;
                        rv++;
                        }
                else
                        {
                        break;
                        }
                }
        }
return rv;
}


unsigned char TMCreateWrireStream::WriteShort(unsigned short datas)
{
return Write((char*)&datas,2);
}


unsigned char TMCreateWrireStream::WriteLong(unsigned long datas)
{
return Write((char*)&datas,4);
}


unsigned int TMCreateWrireStream::WriteFileX (char *lpFileName)
{
unsigned int rv=0;
if (lpFileName)
        {
        TMCreateReadStream ReadStr;
        if (ReadStr.OpenStream(lpFileName))
                {
                int curfsize=ReadStr.GetFileSize();
                if (curfsize)
                        {
                        while (curfsize)
                                {
                                if (WriteByte(ReadStr.ReadByteBX()))
                                        {
                                        curfsize--;
                                        rv++;
                                        }
                                else
                                        {
                                        break;
                                        }
                                }
                        }
                ReadStr.CloseStream();
                }
        }
return rv;
}




unsigned long TMCreateWrireStream::CopyFile (char *lpInputFN, char *lpOutFN)
{
unsigned long rv = 0;
unsigned char DataBuf[512];
HANDLE Hndl_Wr = CreateFile_E (lpOutFN);
HANDLE Hndl_Rd = OpenFileToRead_E (lpInputFN);
if (Hndl_Wr != INVALID_HANDLE_VALUE && Hndl_Rd != INVALID_HANDLE_VALUE)
    {
    unsigned long InSize = GetSizeFile_E (Hndl_Rd);
    unsigned long FILESIZE = InSize;
    if (InSize)
        {
        unsigned long BlockSize;
        while (InSize)
            {
            if (InSize >= sizeof(DataBuf))
                {
                BlockSize = sizeof(DataBuf);
                }
            else
                {
                BlockSize = InSize;
                }
            unsigned long curlen_rd = ReadFileE (DataBuf, BlockSize, Hndl_Rd);
            if (curlen_rd != BlockSize) break;
            unsigned long curlen_wr = WriteFile_E ((char*)DataBuf, BlockSize, Hndl_Wr);
            if (curlen_wr != BlockSize) break;
            InSize = InSize - BlockSize;
            }
        if (!InSize) rv = FILESIZE;
        }
    }
if (Hndl_Wr != INVALID_HANDLE_VALUE) CloseFile_E (Hndl_Wr);
if (Hndl_Rd != INVALID_HANDLE_VALUE) CloseFile_E (Hndl_Rd);

return rv;
}





char TMCreateWrireStream::WriteString (TDString mstr)
{
char datas,vrt=0;
char *lpRam = (char*)mstr.c_str();
while (1)
        {
        datas=lpRam[0];
        if (!datas)
                {
                vrt=1;
                break;
                }
        if (!WriteByte(datas)) break;
        lpRam++;
        }
return vrt;
}



char TMCreateWrireStream::WriteASCIIZ (char *lpStr)
{
char rv=0,datas;
if (lpStr)
	{
	while (1)
		{
		datas=lpStr[0];
		if (!datas)
			{
			rv=1;
			break;
			}
		if (!WriteByte(datas)) break;
		lpStr++;
		}
	}
return rv;
}



char TMCreateWrireStream::WriteTextIniStringParam (char *lpNameVariable,char *lpParam)
{
char rv=0;
if (lpNameVariable && lpParam)
	{
	int lenvarname = TSTMSTRING::lenstr (lpNameVariable);
	int lenparam = TSTMSTRING::lenstr (lpParam);
	if (lenvarname && lenparam)
		{
		if (WriteASCIIZ (lpNameVariable))
			{
			if (WriteByte ('='))
				{
				if (WriteASCIIZ (lpParam))
					{
					if (WriteByte (13))
						{
						if (WriteByte (10))
							{
							rv=1;
							}
						}
					}
				}
			}
		}
	}
return rv;
}


char TMCreateWrireStream::WriteTextIniIntParam (char *lpNameVariable,int paramint)
{
char rv=0;
if (lpNameVariable)
	{
	int lenvarname = TSTMSTRING::lenstr (lpNameVariable);
	TDString datas = TSTMSTRING::LongToStr((long)paramint);
	if (lenvarname)
		{
		if (WriteASCIIZ (lpNameVariable))
			{
			if (WriteByte ('='))
				{
				if (WriteASCIIZ ((char*)datas.c_str()))
					{
					if (WriteByte (13))
						{
						if (WriteByte (10))
							{
							rv=1;
							}
						}
					}
				}
			}
		}
	}
return rv;
}


uint32_t TMCreateWrireStream::ReadData (void *lp,uint32_t size)
{
uint32_t rv = ReadFileE (lp, size,Handll);
return rv;
}





