#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "cmd_GetINI.h"

extern LPWSTR iniPath;

UINT __fastcall GetProfile_ReadFontSize()
{
	if (iniPath) {
		return GetPrivateProfileIntW(L"ModOption", L"FontSize", 0, iniPath);
	}

	return 0;
}
