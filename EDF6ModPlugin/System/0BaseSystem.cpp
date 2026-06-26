#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "../cmd/cmd_GetINI.h"
#include "screen/0D3D11.h"
#include "1GameFunc_Model.h"

#include "0BaseSystem.h"

void __fastcall GameSystem_HookFunction(PBYTE hmodDLL) {
	HookFunction_D3D11(hmodDLL);
	GameSystem_HookFunction_Common(hmodDLL);
	GetGameFunction_Model(hmodDLL);
}

extern "C" {
	void __fastcall ASMGameSetFont();
	uintptr_t GameSetFont_RetAddr;

	void __fastcall ASMGameLoadVoiceFiles();
	uintptr_t _Call_GameLoadVoiceFile;
}

void __fastcall GameSystem_HookFunction_Common(PBYTE hmodDLL)
{
	// edf.dll+1181AAE
	// find <AddFontMemResourceEx>, from mov eax, dword ptr [rbx]
	int i_GameSetFont = 0x1181AAE;
	hookGameBlockWithInt3((void*)(hmodDLL + i_GameSetFont), (uintptr_t)ASMGameSetFont);
	WriteHookToProcess((void*)(hmodDLL + i_GameSetFont + 15), (void*)&nop1, 1U);
	GameSetFont_RetAddr = (uintptr_t)(hmodDLL + i_GameSetFont + 16);

	// edf.dll+7B2D50
	// find 483BBEE000000075AB488B5C2430488B6C2438488B7424404883C4205FC3
	int i_GameLoadVoiceFiles = 0x7B2D50;
	hookGameBlockWithInt3((void*)(hmodDLL + i_GameLoadVoiceFiles), (uintptr_t)ASMGameLoadVoiceFiles);
	WriteHookToProcess((void*)(hmodDLL + i_GameLoadVoiceFiles + 15), (void*)&nop1, 1U);
	// edf.dll+7B2500
	// tikyuu6_voice_vtuber.awb 's call
	_Call_GameLoadVoiceFile = (uintptr_t)(hmodDLL + 0x7B2500);

}

float __fastcall GameSetFont_GetNewFontSize(int* pOldSize, UINT32 charset)
{
	float fontSize = 32;
	auto iniFontSize = INIConfig_ReadFontSize();
	if (charset == GB2312_CHARSET || charset == CHINESEBIG5_CHARSET) // 0x86 and 0x88
	{
		// 32 and 36 will cause Planet Sniper Cannon's damage to start a new line
		switch (iniFontSize) {
		case 1:
			fontSize = 44;
			break;
		case 2:
			fontSize = 40;
			break;
		case 3:
			return fontSize;
		case 4:
			fontSize = 28;
			break;
		case 5:
			fontSize = 18;
			break;
		default:
			fontSize = 36;
			break;
		}

		*pOldSize = fontSize;
		return fontSize;
	}

	float scaleLevel;
	switch (iniFontSize) {
	case 1:
		fontSize = 40;
		break;
	case 2:
		fontSize = 36;
		break;
	case 3:
		fontSize = 28;
		break;
	case 4:
		fontSize = 24;
		break;
	case 5:
		fontSize = 16;
		break;
	default:
		return fontSize;
	}

	*pOldSize = fontSize;
	return fontSize;
	// LOGFONTW
}

void __fastcall LoadNewVoiceFilesCPP(void* pAudio) {
	typedef void(__fastcall* CallFunc)(void* pAudio, LPCWSTR FileName, int r8d);
	auto Call_GameLoadVoiceFile = (CallFunc)_Call_GameLoadVoiceFile;

	if (std::filesystem::exists(L"./sound/pc/add_audio41_main.awb")) {
		Call_GameLoadVoiceFile(pAudio, L"app:/sound/adx/add_audio41_main.awb", 0);
	}

	if (std::filesystem::exists(L"./sound/pc/add_audio6_dlc1.awb")) {
		Call_GameLoadVoiceFile(pAudio, L"app:/sound/adx/add_audio6_dlc1.awb", 0);
	}
	// end
}
