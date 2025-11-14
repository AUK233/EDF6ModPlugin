#pragma once

typedef struct checkPatchGame_t {
	const wchar_t* TitleText;
	int TitleSize;
} *PcheckPatchGame;

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

const wchar_t wstr_GameTitleMod[] = L"EDF6 for PC in Mod Mode";

extern "C" {
	uintptr_t edfCreateWSting;
	//
	wchar_t wstr_GameTitlePatch[] = L"EarthDefenceForce 6 for Patch Mode";
	void __fastcall ASMsetGameTitlePatch();
	uintptr_t setGameTitleRetAddr;

	const wchar_t* SetGameTitleModText;
	int SetGameTitleModSize;
	void __fastcall ASMsetGameTitleMod();
	//
	void __fastcall ASMupdateOnlineRoomMissionName();
	uintptr_t updateOnlineRoomMissionNameRetAddr;
}

static PBYTE hDllnoMod;

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
	if (*(INT64*)(hDllnoMod + protectAddress) == 8397322248735459656) {
		//MessageBox(NULL, L"No need for protection", L"Debug", MB_OK);
		return;
	}
	// EDF.dll+741c0, here?
	static unsigned char originalValue[] = {
		0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x74, 0x24, 0x20, 0x55, 0x57, 0x41, 0x56, 0x48, 0x8B,
		0xEC, 0x48, 0x83, 0xEC, 0x70
	};
	WriteHookToProcess((void*)(hDllnoMod + protectAddress), originalValue, 21U);

	FlushInstructionCache(hDllnoMod, NULL, 0);
}

int checkPatchGame(PBYTE hmodDLL, PcheckPatchGame pMod) {
	// edf.dll+3D450
	if (wcscmp((wchar_t*)(hmodDLL + 0x17E8510), L"EarthDefenceForce 6 for PC")) {
		return 0;
	}
	hDllnoMod = hmodDLL;

	// edf.dll+703173, skip epic check
	// find "Eos_FailedCrossplay_Caption", is test rax, rax je
	if (CheckSkipEOS())
	{
		// skip eos
		BYTE skipEOS[] = {
			0xE9, 0x9C, 0x02, 0x00, 0x00,
			0x90
		};
		WriteHookToProcess((void*)(hmodDLL + 0x703173), skipEOS, 6U);
	}

	BYTE vp_nop2[] = { 0x66, 0x90 };
	// patch game title
	edfCreateWSting = (uintptr_t)(hmodDLL + 0x3D450);
	// edf.dll+70069C
	int titleFuncAddress = 0x70069C;
	if (pMod) {
		SetGameTitleModText = pMod->TitleText;
		SetGameTitleModSize = pMod->TitleSize;
		hookGameBlockWithInt3((void*)(hmodDLL + titleFuncAddress), (uintptr_t)ASMsetGameTitleMod);
	}
	else {
		hookGameBlockWithInt3((void*)(hmodDLL + titleFuncAddress), (uintptr_t)ASMsetGameTitlePatch);
	}
	WriteHookToProcess((void*)(hmodDLL + titleFuncAddress + 15), vp_nop2, 2U);
	setGameTitleRetAddr = (uintptr_t)(hmodDLL + titleFuncAddress + 22);

	// edf.dll+8EB800, find "Online_NotHaveContents"
	int OnlineRoomAddress = 0x8EB800;
	hookGameBlockWithInt3((void*)(hmodDLL + OnlineRoomAddress), (uintptr_t)ASMupdateOnlineRoomMissionName);
	updateOnlineRoomMissionNameRetAddr = (uintptr_t)(hmodDLL + OnlineRoomAddress + 15);

	return 6;
}
