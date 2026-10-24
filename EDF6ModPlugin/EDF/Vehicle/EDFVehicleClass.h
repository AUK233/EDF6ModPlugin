#pragma once
#include "Base/base_baseObject.h"

// edf.dll+629450
typedef struct XGS_VehicleBase_t : XGS_GameObjectBase_t {
	char pad5C0[0x38]; // edf.dll+6294C5
	bool m_bIsMultipleAISeats, pad5f9[7];
	INT64 pad600; // edf.dll+62B3FB, because of this, it cannot be used.
	void* pSeat;
	void* padSeat;
	size_t m_seatCount;
	char pad620[0x60];
	char padRigidBodyContactDamageControl[0x120]; //maybe
	char pad7A0[0xD90];
} *P_XGS_VehicleBase;
#if 1
static_assert(offsetof(XGS_VehicleBase_t, m_bIsMultipleAISeats) == 0x5F8);
static_assert(offsetof(XGS_VehicleBase_t, pSeat) == 0x608);
static_assert(sizeof(XGS_VehicleBase_t) == 0x1530);
#endif

// edf.dll+65C540
typedef struct XGS_Vehicle_CarBase_t : XGS_VehicleBase_t {
	char pad1530[0xC0];
	XGS_StateMachineTable_t m_stateMachine;
	char pad1620[0x1340];
} *P_XGS_Vehicle_CarBase;
#if 1
static_assert(offsetof(XGS_Vehicle_CarBase_t, m_stateMachine) == 0x15F0);
static_assert(sizeof(XGS_Vehicle_CarBase_t) == 0x2960);
#endif

// edf.dll+67E180
typedef struct XGS_Vehicle_TankBase_t : XGS_Vehicle_CarBase_t {
	char pad2960[0x140];
} *P_XGS_Vehicle_TankBase;
#if 1
static_assert(sizeof(XGS_Vehicle_TankBase_t) == 0x2AA0);
#endif


void __fastcall EDF_HookFunction_Vehicle(PBYTE hmodDLL);

void __fastcall VehicleVFT190_SetAISeat(void* pThis, void* rdx, int muliSeat);
void __fastcall VehicleVFT230_CheckSeatIsAllowShooting(void* pThis, int seatIndex, void* r8);
