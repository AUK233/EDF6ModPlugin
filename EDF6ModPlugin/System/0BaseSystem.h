#pragma once

void __fastcall GameSystem_HookFunction(PBYTE hmodDLL);
void __fastcall GameSystem_HookFunction_Common(PBYTE hmodDLL);

extern "C" {
	float __fastcall GameSetFont_GetNewFontSize(int* pOldSize, UINT32 charset);
	void __fastcall LoadNewVoiceFilesCPP(void* pAudio);
}
