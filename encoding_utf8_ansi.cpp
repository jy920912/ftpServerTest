//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "encoding_utf8_ansi.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
AnsiString __fastcall UTF8ToANSI(const char *pszCode)			//UTF8 to ANSI
{
	BSTR    bstrWide;
	char*   pszAnsi;
	int     nLength;
	AnsiString result;

	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1, NULL, NULL);
	bstrWide = SysAllocStringLen(NULL, nLength);

	MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen(pszCode) + 1, bstrWide, nLength);

	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
	pszAnsi = new char[nLength];

	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
	SysFreeString(bstrWide);

	result = pszAnsi;
	delete(pszAnsi);
	return result;
}
AnsiString __fastcall ANSIToUTF8(const char * pszCode)  		//ANSI to UTF8
{
	int		nLength, nLength2;
	BSTR	bstrCode;
	char*	pszUTFCode = NULL;
	AnsiString result;

	nLength = MultiByteToWideChar(CP_ACP, 0, pszCode, lstrlen(pszCode), NULL, NULL);
	bstrCode = SysAllocStringLen(NULL, nLength);
	MultiByteToWideChar(CP_ACP, 0, pszCode, lstrlen(pszCode), bstrCode, nLength);

	nLength2 = WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, 0, NULL, NULL);
	pszUTFCode = new char[nLength2];
	WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, nLength2, NULL, NULL);

	result = pszUTFCode;
	delete pszUTFCode;
	return result;
}