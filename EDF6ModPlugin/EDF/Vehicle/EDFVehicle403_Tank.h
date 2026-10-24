#pragma once
#include "EDFVehicleClass.h"

// edf.dll+5FE270
typedef struct XGS_Vehicle403_Tank_t : XGS_Vehicle_TankBase_t {
	char padRailgunSlot[0x10][3]; // edf.dll+5FE2D0
	char pad2AD0[0x50];
} *P_XGS_Vehicle403_Tank;
#if 1
static_assert(offsetof(XGS_Vehicle403_Tank_t, padRailgunSlot) == 0x2AA0);
static_assert(sizeof(XGS_Vehicle403_Tank_t) == 0x2B20);
#endif

typedef struct NEW_Vehicle403_Tank_t : XGS_Vehicle403_Tank_t {
} *P_NEW_Vehicle403_Tank;
#if 1
static_assert(offsetof(NEW_Vehicle403_Tank_t, padRailgunSlot) == 0x2AA0);
#endif

void __fastcall EDF_HookFunction_Vehicle403_Tank(PBYTE hmodDLL);

extern "C" {
	void __fastcall Vehicle403_Tank_Initialization(P_NEW_Vehicle403_Tank pIn);
}

void __fastcall EDF_Vehicle403_Tank_SetAISeat190(P_NEW_Vehicle403_Tank pThis, void* rdx);
void __fastcall EDF_Vehicle403_Tank_CheckSeatIsAllowShooting230(P_NEW_Vehicle403_Tank pThis, int seatIndex, void* r8);
