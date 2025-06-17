#include "pch.h"

#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

typedef void(WINAPI* EDFImportDLL)();
EDFImportDLL EDF_CPP_OnBoot;
PBYTE hmodDLL;
WCHAR iniEpic[MAX_PATH];

extern "C" __declspec(dllexport) void CPP_OnBoot() {
	timeBeginPeriod(1);
	EDF_CPP_OnBoot();
	timeEndPeriod(1);
};

// Injects hook into game process
void __fastcall WriteHookToProcess(void* addr, void* data, size_t len) {
	DWORD oldProtect;
	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(addr, data, len);
	VirtualProtect(addr, len, oldProtect, &oldProtect);
}

// update game's original functions with interruption, need 15 bytes
void __fastcall hookGameBlockWithInt3(void* targetAddr, uint64_t dataAddr) {
	uint8_t hookFunction[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// jmp
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // addr
		0xCC											// int3
	};
	memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

int CheckSkipEOS() {
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, "LaunchGame.ini", "r");
	if (err || !file) {
		return 0;
	}

	char line[256];
	int value = 0;

	while (fgets(line, sizeof(line), file)) {
		line[strcspn(line, "\r\n")] = '\0';

		if (!strncmp(line, "NoOperation=", 12)) {
			value = atoi(line + 12);
			break;
		}
	}

	fclose(file);
	return value;
}

constexpr unsigned char nop2[] = { 0x66, 0x90 };

extern "C" {
	uintptr_t edfCreateWSting;
	//
	wchar_t wstr_GameTitle[] = L"EarthDefenceForce 6 for Patch Mode";
	void __fastcall ASMsetGameTitle();
	uintptr_t setGameTitleRetAddr;
	//
	void __fastcall ASMupdateOnlineRoomMissionName();
	uintptr_t updateOnlineRoomMissionNameRetAddr;
}

extern "C" void __fastcall ProtectEDFCreateWSting() {
	// edf.dll+3D450
	/*static unsigned char originalValue[] = {
		0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57,
		0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x71, 0x18, 0x4D, 0x8B, 0xF0, 0x4C,
		0x8B, 0xFA, 0x48, 0x8B, 0xF9, 0x4C, 0x3B, 0xC6, 0x77, 0x2A, 0x48, 0x8B, 0xE9, 0x48, 0x83, 0xFE,
		0x08
	};
	WriteHookToProcess((void*)edfCreateWSting, originalValue, 49U);*/

	// find 48895C2418488974242055574156488BEC4883EC70488B057CAEF7014833C4488945F8498BF0488BFA488BD9488B01
	int protectAddress = 0x741C0;
	if (*(INT64*)(hmodDLL + protectAddress) == 8397322248735459656) {
		//MessageBox(NULL, L"No need for protection", L"Debug", MB_OK);
		return;
	}
	// EDF.dll+741c0, here?
	static unsigned char originalValue[] = {
		0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x55, 0x57, 0x41, 0x56, 0x48, 0x8B,
		0xEC, 0x48, 0x83, 0xEC, 0x70
	};
	WriteHookToProcess((void*)(hmodDLL + protectAddress), originalValue, 21U);

	FlushInstructionCache(hmodDLL, NULL, 0);
}

void patchGame() {

	if (CheckSkipEOS())
	{
		BYTE skipEOS[] = {
			0xE9, 0x9C, 0x02, 0x00, 0x00,
			0x90
		};
		WriteHookToProcess((void*)(hmodDLL + 0x703173), skipEOS, 6U);
	}

	// patch game title
	edfCreateWSting = (uintptr_t)(hmodDLL + 0x3D450);
	// edf.dll+70069C
	int titleFuncAddress = 0x70069C;
	hookGameBlockWithInt3((void*)(hmodDLL + titleFuncAddress), (uintptr_t)ASMsetGameTitle);
	WriteHookToProcess((void*)(hmodDLL + titleFuncAddress + 15), (void*)&nop2, 2U);
	setGameTitleRetAddr = (uintptr_t)(hmodDLL + titleFuncAddress + 22);

	// edf.dll+8EB800, find "Online_NotHaveContents"
	int OnlineRoomAddress = 0x8EB800;
	hookGameBlockWithInt3((void*)(hmodDLL + OnlineRoomAddress), (uintptr_t)ASMupdateOnlineRoomMissionName);
	updateOnlineRoomMissionNameRetAddr = (uintptr_t)(hmodDLL + OnlineRoomAddress + 15);
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

		auto moduleHandle = LoadLibraryW(L"edf.dll");
		if (moduleHandle != nullptr)
		{
			EDF_CPP_OnBoot = (EDFImportDLL)GetProcAddress(moduleHandle, "CPP_OnBoot");

			hmodDLL = (PBYTE)moduleHandle;

			if ( !wcscmp((wchar_t*)(hmodDLL + 0x17E8510), L"EarthDefenceForce 6 for PC") ) {
				// edf.dll+3D450
				patchGame();
			}
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

