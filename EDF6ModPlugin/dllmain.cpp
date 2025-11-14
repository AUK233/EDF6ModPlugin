#include "pch.h"
//#include "framework.h"

#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "utiliy.h"
#include "commonNOP.h"

#include "../patch/patchMain.hpp"
#include "updateMissionPack.hpp"
#include "updateMainSystem.hpp"

#include "cmd/cmd_hotkey.h"

typedef void(WINAPI* EDFImportDLL)();
EDFImportDLL EDF_CPP_OnBoot;
PBYTE hmodDLL;
WCHAR iniPath[MAX_PATH];

// ini content

extern "C" __declspec(dllexport) void CPP_OnBoot() {
	static int initialized = 0;
	if (!initialized) {
		initialized = 6;

		UINT testDLL = 0;
		if (std::filesystem::exists("./modtest/exa/config.sgo")) {
			//testDLL = GetPrivateProfileIntW(L"ModOption", L"testDLL", 0, iniPath);
			// read absolute path
			testDLL = GetPrivateProfileIntW(L"ModOption", L"testDLL", 0,
				L"I:\\SteamLibrary\\steamapps\\common\\EARTH DEFENSE FORCE 6\\mod.ini");

			if (GetPrivateProfileIntW(L"ModOption", L"cheat", 0, iniPath)) {
				cmd_ModConsole_Initialize(hmodDLL);
				HANDLE tempHND = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)cmd_ModConsole_MonitorKeys, NULL, NULL, NULL);
				if (tempHND) {
					CloseHandle(tempHND);
				}
			}
		}

		checkPatchGame_t ModMode;
		if (testDLL) {
			ModMode.TitleText = L"EDF6 is Debug Mod Mode";
			ModMode.TitleSize = 22;
		}
		else {
			ModMode.TitleText = L"EDF6 for PC in Mod Mode";
			ModMode.TitleSize = 23;
		}

		if (!checkPatchGame(hmodDLL, &ModMode)) {
			goto RealFunction;
		}

		hook_updateMainSystem(hmodDLL, testDLL);

		//
		hook_updateMissionPack(hmodDLL);
		/*auto testModule = LoadLibraryW(L"mppp.dll");
		typedef bool(__fastcall* LoadDef)(void*);
		auto test_func = (LoadDef)GetProcAddress(testModule, "EML6_Load");
		test_func(0);*/
	}

	RealFunction:
	timeBeginPeriod(1);
	EDF_CPP_OnBoot();
	timeEndPeriod(1);
};

extern "C" {
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

			// find 48 8B 02 C6 80 B6 08 00 00 01 48 83 C2 08
			/*hookGameBlockWithInt3((void*)(hmodDLL + 0x5A44E6), (uintptr_t)ASMsetPlayerWeaponFriendlyFire);
			setPlayerWeaponFriendlyFireRetAddr = (uintptr_t)(hmodDLL + 0x5A4525);*/

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

