#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "cmd_GetINI.h"

extern LPWSTR iniPath;

extern "C" {
	int Config_FullAO; // 0 is no, 1 is open
	int Config_PostProcess; // 0 is no, 1 is open
	int Config_DLAA; // 0 is no, 1 is open
}

void __fastcall INIConfig_Initialize() {
	Config_FullAO = INIConfig_ReadFullAO();
	Config_PostProcess = GetPrivateProfileIntW(L"Image", L"PostProcess", 0, iniPath);
	Config_DLAA = GetPrivateProfileIntW(L"Image", L"DLAA", 0, iniPath);
}

UINT __fastcall INIConfig_ReadFontSize() {
	if (iniPath) {
		return GetPrivateProfileIntW(L"ModOption", L"FontSize", 0, iniPath);
	}

	return 0;
}

UINT __fastcall INIConfig_ReadFullAO() {
	return GetPrivateProfileIntW(L"Image", L"FullAO", 0, iniPath);
}
