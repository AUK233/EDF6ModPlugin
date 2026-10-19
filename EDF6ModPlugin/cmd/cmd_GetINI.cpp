#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "cmd_GetINI.h"

extern LPWSTR iniPath;

extern "C" {
	int Config_FullAO; // 0 is no, 1 is open
	int Config_PostProcess; // 0 is no, 1 is open
	int Config_PostProcessTexIndex;
	int Config_DLSS; // 0 is no, 1-6
	int Config_DLSSPreset;
	int Config_OnDX12; // 0 is no, 1 is open
	int Config_DLSSFG; // 0 is no, 1 is open
}

void __fastcall INIConfig_Initialize() {
	Config_FullAO = INIConfig_ReadFullAO();

	Config_PostProcess = GetPrivateProfileIntW(L"Graphic", L"PostProcess", 0, iniPath);
	Config_PostProcessTexIndex = GetPrivateProfileIntW(L"Graphic", L"PostProcessTexIndex", 0, iniPath);
	Config_DLSS = GetPrivateProfileIntW(L"Graphic", L"DLSS", 0, iniPath);
	Config_DLSSPreset = GetPrivateProfileIntW(L"Graphic", L"DLSSPreset", 0, iniPath);
	Config_OnDX12 = GetPrivateProfileIntW(L"Graphic", L"OnDX12", 0, iniPath);
	Config_DLSSFG = GetPrivateProfileIntW(L"Graphic", L"DLSSFG", 0, iniPath);
}

UINT __fastcall INIConfig_ReadFontSize() {
	if (iniPath) {
		return GetPrivateProfileIntW(L"ModOption", L"FontSize", 0, iniPath);
	}

	return 0;
}

UINT __fastcall INIConfig_ReadFullAO() {
	return GetPrivateProfileIntW(L"Graphic", L"FullAO", 0, iniPath);
}
