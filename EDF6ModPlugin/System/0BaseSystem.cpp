#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "../cmd/cmd_GetINI.h"
#include "0BaseSystem.h"

void __fastcall GameSystem_HookFunction(PBYTE hmodDLL)
{
	GameSystem_HookFunction_Common(hmodDLL);
}

extern "C" {
	void __fastcall ASMGameSetFont();
	uintptr_t GameSetFont_RetAddr;
}

void __fastcall GameSystem_HookFunction_Common(PBYTE hmodDLL)
{
	// edf.dll+1181AAE
	// find <AddFontMemResourceEx>, from mov eax, dword ptr [rbx]
	int i_GameSetFont = 0x1181AAE;
	hookGameBlockWithInt3((void*)(hmodDLL + i_GameSetFont), (uintptr_t)ASMGameSetFont);
	WriteHookToProcess((void*)(hmodDLL + i_GameSetFont + 15), (void*)&nop1, 1U);
	GameSetFont_RetAddr = (uintptr_t)(hmodDLL + i_GameSetFont + 16);
}

float __fastcall GameSetFont_GetNewFontSize(int* pOldSize, UINT32 charset)
{
	float fontSize = 32;
	auto iniFontSize = GetProfile_ReadFontSize();
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
