#include "pch.h"
//#include "framework.h"

#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "utiliy.h"
#include "commonNOP.h"

#include "updateMissionPack.hpp"
#include "updateMainSystem.hpp"

typedef void(WINAPI* EDFImportDLL)();
EDFImportDLL EDF_CPP_OnBoot;
PBYTE hmodDLL;
WCHAR iniPath[MAX_PATH];

extern "C" __declspec(dllexport) void CPP_OnBoot() {
	timeBeginPeriod(1);
	EDF_CPP_OnBoot();
	timeEndPeriod(1);
};

extern "C" {
	void __fastcall ASMreadMissionSavaData();
	uintptr_t readMissionSavaDataRetAddr;
	void __fastcall ASMsetPlayerWeaponFriendlyFire();
	uintptr_t setPlayerWeaponFriendlyFireRetAddr;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);

		//csharp_RequestBoot
		//csharp_RequestExit
		auto moduleHandle = LoadLibraryW(L"edf.dll");
		if (moduleHandle != nullptr)
		{
			EDF_CPP_OnBoot = (EDFImportDLL)GetProcAddress(moduleHandle, "CPP_OnBoot");

			hmodDLL = (PBYTE)moduleHandle;

			GetModuleFileNameW(hModule, iniPath, _countof(iniPath));
			PathRemoveFileSpecW(iniPath);
			wcscat_s(iniPath, L"\\mod.ini");
			//UINT testDLL = GetPrivateProfileIntW(L"ModOption", L"testDLL", 0, iniPath);

			// find PackageName function.
			// need fix
			// edf.dll+E2970, Determine mission savedata location, R8 is 1 when it is online.
			//hookGameBlockWithInt3((void*)(hmodDLL + 0xE2970), (uintptr_t)ASMreadMissionSavaData);
			//readMissionSavaDataRetAddr = (uintptr_t)(hmodDLL + 0xE2980);

			// need fix
			//WriteHookToProcess(hmodDLL + 0x175F140, (void*)L"./1mod.cpk", 22U);
			//WriteHookToProcess(hmodDLL + 0x17E5408, (void*)L"1mod.cpk", 18U);
			WriteHookToProcess(hmodDLL + 0x1767B70, (void*)L"./test/config.sgo", 36U);


			// find 48 8B 02 C6 80 B6 08 00 00 01 48 83 C2 08
			/*hookGameBlockWithInt3((void*)(hmodDLL + 0x5A44E6), (uintptr_t)ASMsetPlayerWeaponFriendlyFire);
			setPlayerWeaponFriendlyFireRetAddr = (uintptr_t)(hmodDLL + 0x5A4525);*/

			

			hook_updateMainSystem(hmodDLL, 0);

			//
			hook_updateMissionPack(hmodDLL);
			/*auto testModule = LoadLibraryW(L"mppp.dll");
			typedef bool(__fastcall* LoadDef)(void*);
			auto test_func = (LoadDef)GetProcAddress(testModule, "EML6_Load");
			test_func(0);*/
		}
		else {
			// to crash!
			*(int*)0 = 0;
		}

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

