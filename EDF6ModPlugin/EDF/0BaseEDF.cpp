#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "0BaseEDF.h"
#include "Class/EDFSoldierHeavyArmor.h"

void __fastcall HookFunction_EDFseries(PBYTE hmodDLL) {
	HookFunction_EDFCommonFunction(hmodDLL);
	EDF_HookFunction_HeavyArmor(hmodDLL);
}

extern "C" {
	void __fastcall ASMhudShowSupportSlot();
	uintptr_t hudShowSupportSlotRetAddr;
}

void __fastcall HookFunction_EDFCommonFunction(PBYTE hmodDLL) {

	// EDF.dll+82E580, find "lyt_HudSubWeaponGuageR"
	int i_hudShowSupportSlot = 0x82E580;
	hookGameBlockWithInt3((void*)(hmodDLL + i_hudShowSupportSlot), (uintptr_t)ASMhudShowSupportSlot);
	WriteHookToProcess((void*)(hmodDLL + i_hudShowSupportSlot + 15), (void*)&nop3, 3U);
	hudShowSupportSlotRetAddr = (uintptr_t)(hmodDLL + i_hudShowSupportSlot + 18);
}
