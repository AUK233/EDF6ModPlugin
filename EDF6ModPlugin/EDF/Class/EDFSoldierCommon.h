#pragma once
#include "EDF/EDFSoldierClass.h"


void __fastcall EDF_HookFunction_ClassCommon(PBYTE hmodDLL);

void __fastcall EDF_SoldierCommon_SetJump220(P_XGS_SoldierBase pIn, int state);
bool __fastcall EDF_SoldierCommon_ResetActionStatus(P_XGS_SoldierBase pIn, void* pRDX);
void __fastcall EDF_SoldierCommon_SetAccessoryEffects(P_XGS_SoldierBase pIn);

enum class SoldierAccessoryValueFetchType : int {
	DirectReturn = 0,
	TakeMaximum = 1,
	TakeMinimum = 2,
	DefaultAddAcquired = 3,
	DefaultMultiplyAcquired = 4
};

float __fastcall EDF_SoldierCommon_GetAccessoryValueFP32(P_XGS_SoldierBase pIn, UINT32 accessoryType, float defaultValue, SoldierAccessoryValueFetchType fetchType);
int __fastcall EDF_SoldierCommon_GetAccessoryValueINT32(P_XGS_SoldierBase pIn, UINT32 accessoryType, int defaultValue, SoldierAccessoryValueFetchType fetchType);
