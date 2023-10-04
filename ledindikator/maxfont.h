//#pragma once
#ifndef MMFNTT_
#define MMFNTT_

#pragma pack(push, 1)


// ��������� �����


// �������� ����������� ������
struct MaxChar
{
	int Width;	// ������ ������� � ��������
};


typedef struct {
	unsigned char Width;	// ������ ������� � ��������
} MicroFWidth;


// ��������� ��� � ������ �����
struct MaxFont
{
	char Label[8];							// ���������� ����� "MaxFonts"
	int Height;								// ������ ���� �������� ������� ������  � ��������.
	int OffsetFromBeginFileToMaxChar[256];	// ����� �������� �� ������ ����� �������� MaxChar.
};



typedef struct {
	unsigned long Height;		       // ������ ���� �������� ������� ������  � ��������.
	unsigned long OffsetBitMask[256];      // ����� �������� �� ������ ����� �������� MaxChar.
} MaxFontMicro;
#pragma pack(pop)

#endif


