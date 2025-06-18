#include "pch.h"
#include "PluginAPI.h"

#include "updateMissionPack1.hpp"
#include "updateMissionPack.hpp"

extern "C" void __declspec(dllexport) LoadMPPP() {
	auto hmodDLL = GetModuleHandleW(L"EDF.dll");
	hook_updateMissionPackTRUE((PBYTE)hmodDLL);
}

extern "C" BOOL __declspec(dllexport) EML6_Load(PluginInfo* pluginInfo) {
	auto hmodDLL = GetModuleHandleW(L"EDF.dll");
	hook_updateMissionPackTRUE((PBYTE)hmodDLL);
	hook_updateMissionPack((PBYTE)hmodDLL);

	if (!pluginInfo) {
		return false;
	}

	pluginInfo->infoVersion = PluginInfo::MaxInfoVer;
	pluginInfo->name = "EDF6 mission pack plus plus";
	pluginInfo->version = PLUG_VER(1, 0, 2, 0);

	return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		DisableThreadLibraryCalls(hModule);

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

