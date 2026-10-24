#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Class/EDFSoldierCommon.h"
#include "Class/EDFSoldierAssultSoldier.h"
#include "Class/EDFSoldierHeavyArmor.h"

#include "Vehicle/EDFVehicle403_Tank.h"

#include "0BaseEDF.h"

void __fastcall HookFunction_EDFseries(PBYTE hmodDLL) {
	HookFunction_EDFCommonFunction(hmodDLL);
	EDF_HookFunction_ClassCommon(hmodDLL);
	EDF_HookFunction_AssultSoldier(hmodDLL);
	EDF_HookFunction_HeavyArmor(hmodDLL);

	EDF_HookFunction_Vehicle(hmodDLL);
	EDF_HookFunction_Vehicle403_Tank(hmodDLL);
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

	// edf.dll+826AA5
	int MaxHPdisplay = 9999999;
	WriteHookToProcess((void*)(hmodDLL + 0x826AA5 + 1), &MaxHPdisplay, 4U);
}
