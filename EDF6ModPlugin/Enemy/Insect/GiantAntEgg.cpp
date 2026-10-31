#include "pch.h"
#include "utiliy.h"
#include "commonNOP.h"

#include "GiantAntEgg.h"

extern "C" {
	void __fastcall ASMGiantAntEggToBreak();
	uintptr_t GiantAntEggToBreakRetAddr;
}

void __fastcall Enemy_HookFunction_GiantAntEgg(PBYTE hmodDLL) {
	// edf.dll+4146F6
	int i_EggBreak = 0x4146F6;
	hookGameBlockWithInt3((void*)(hmodDLL + i_EggBreak), (uintptr_t)ASMGiantAntEggToBreak);
	GiantAntEggToBreakRetAddr = (uintptr_t)(hmodDLL + i_EggBreak + 26);
}
