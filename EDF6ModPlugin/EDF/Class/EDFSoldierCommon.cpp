#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "EDFSoldierCommon.h"

extern "C" {
	uintptr_t eHuman_SetJump220; // edf.dll+57A1C0
	uintptr_t eResetSoldierActionStatus59F120; // edf.dll+59F120
	uintptr_t eSoldierSetAccessoryEffects; // edf.dll+599A30

	uintptr_t eSoldierGetAccessoryValueINT32; // edf.dll+592A70
	uintptr_t eSoldierGetAccessoryValueFP32; // edf.dll+592940
}

void __fastcall EDF_HookFunction_ClassCommon(PBYTE hmodDLL) {
	eHuman_SetJump220 = (uintptr_t)(hmodDLL + 0x57A1C0);
	eResetSoldierActionStatus59F120 = (uintptr_t)(hmodDLL + 0x59F120);
	eSoldierSetAccessoryEffects = (uintptr_t)(hmodDLL + 0x599A30);

	eSoldierGetAccessoryValueINT32 = (uintptr_t)(hmodDLL + 0x592A70);
	eSoldierGetAccessoryValueFP32 = (uintptr_t)(hmodDLL + 0x592940);
}

void __fastcall EDF_SoldierCommon_SetJump220(P_XGS_SoldierBase pIn, int state) {
	typedef void(__fastcall* func)(P_XGS_SoldierBase pIn, int state);
	auto fn = (func)eHuman_SetJump220;
	return fn(pIn, state);
}

bool __fastcall EDF_SoldierCommon_ResetActionStatus(P_XGS_SoldierBase pIn, void* pRDX){
	typedef bool(__fastcall* func)(P_XGS_SoldierBase pIn, void* pRDX);
	auto fn = (func)eResetSoldierActionStatus59F120;
	return fn(pIn, pRDX);
}

void __fastcall EDF_SoldierCommon_SetAccessoryEffects(P_XGS_SoldierBase pIn) {
	typedef void(__fastcall* func)(P_XGS_SoldierBase pIn);
	auto fn = (func)eSoldierSetAccessoryEffects;
	fn(pIn);
}

float __fastcall EDF_SoldierCommon_GetAccessoryValueFP32(P_XGS_SoldierBase pIn, UINT32 accessoryType, float defaultValue, SoldierAccessoryValueFetchType fetchType) {
	typedef float(__fastcall* func)(P_XGS_SoldierBase pIn, UINT32 accessoryType, float defaultValue, SoldierAccessoryValueFetchType fetchType);
	auto fn = (func)eSoldierGetAccessoryValueFP32;
	return fn(pIn, accessoryType, defaultValue, fetchType);
}

int __fastcall EDF_SoldierCommon_GetAccessoryValueINT32(P_XGS_SoldierBase pIn, UINT32 accessoryType, int defaultValue, SoldierAccessoryValueFetchType fetchType) {
	typedef int(__fastcall* func)(P_XGS_SoldierBase pIn, UINT32 accessoryType, int defaultValue, SoldierAccessoryValueFetchType fetchType);
	auto fn = (func)eSoldierGetAccessoryValueINT32;
	return fn(pIn, accessoryType, defaultValue, fetchType);
}
