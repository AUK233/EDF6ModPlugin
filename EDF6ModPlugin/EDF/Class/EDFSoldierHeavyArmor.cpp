#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "EDFSoldierClass.hpp"
#include "EDFSoldierHeavyArmor.h"

extern "C" {
	void __fastcall ASMeHeavyArmorInitialization();

	// virtual function
	uintptr_t eHeavyArmorFunc28;
	void __fastcall ASMeHeavyArmorFunc28(PG_HeavyArmor pIn, void* pRDX);
	uintptr_t eHeavyArmorFunc210;
	uintptr_t eHeavyArmorFunc220; // jump
	void __fastcall ASMeHeavyArmorFunc220(PG_HeavyArmor pIn, int iEDX);
}

void __fastcall EDF_HookFunction_HeavyArmor(PBYTE hmodDLL) {

	// EDF.dll+564FB9
	// HeavyArmor old 0x20F0
	int newFencerSize = 0x3000;
	// start:0x20F0, size:0x10, function: swap types.
	// start:0x2100, size:0x10, function: sub weapon.
	WriteHookToProcessCheckECX((void*)(hmodDLL + 0x564FB9 + 1), &newFencerSize, 4U);

	// EDF.dll+565D34
	int i_Initialization = 0x565D34;
	hookGameBlockWithInt3((void*)(hmodDLL + i_Initialization), (uintptr_t)ASMeHeavyArmorInitialization);
	WriteHookToProcess((void*)(hmodDLL + i_Initialization + 15), (void*)&nop3, 3U);

	// vft, EDF.dll+17CF5B8
	auto vft_fencer = (uintptr_t)(hmodDLL + 0x17CF5B8);
	// allow use jump key to activate sub-weapon
	auto vft28 = vft_fencer + 0x28;
	eHeavyArmorFunc28 = *(uintptr_t*)vft28;
	auto newFunc28 = (uintptr_t)ASMeHeavyArmorFunc28;
	WriteHookToProcess((void*)vft28, &newFunc28, 8U);
	//
	auto vft210 = vft_fencer + 0x210;
	eHeavyArmorFunc210 = *(uintptr_t*)vft210;
	auto vft220 = vft_fencer + 0x220;
	eHeavyArmorFunc220 = *(uintptr_t*)vft220;
	auto newFunc220 = (uintptr_t)ASMeHeavyArmorFunc220;
	WriteHookToProcess((void*)vft220, &newFunc220, 8U);
}
