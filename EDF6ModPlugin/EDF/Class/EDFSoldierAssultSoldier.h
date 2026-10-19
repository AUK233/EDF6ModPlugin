#pragma once
#include "EDF/EDFSoldierClass.h"
// ==================================================
// ranegr!
// edf.dll+54FF10
typedef struct XGS_AssultSoldier_t : XGS_SoldierBase_t {
	XGS_StateMachineTable_t m_stateMachine;
	char pad1F20[0x158];
	int m_mapObjectDestroyFlags; // 0 is none, 1 is small, 2 has building
	float f207C;
	char pad2080[0x10];
} *P_XGS_AssultSoldier;
#if 1
static_assert(offsetof(XGS_AssultSoldier_t, m_stateMachine) == 0x1EF0);
static_assert(offsetof(XGS_AssultSoldier_t, m_mapObjectDestroyFlags) == 0x2078);
static_assert(sizeof(XGS_AssultSoldier_t) == 0x2090);
#endif

typedef struct NEW_AssultSoldier_t : XGS_AssultSoldier_t {
	bool m_bIs360Rolling, m_bIsNoDamage;
	float m_rollingDistance;
	int m_rollingMODDuration, m_rollingNoDamageDuration;
} *P_NEW_AssultSoldier;
#if 1
static_assert(offsetof(NEW_AssultSoldier_t, m_bIs360Rolling) == 0x2090);
static_assert(offsetof(NEW_AssultSoldier_t, m_rollingDistance) == 0x2094);
static_assert(offsetof(NEW_AssultSoldier_t, m_rollingMODDuration) == 0x2098);
static_assert(offsetof(NEW_AssultSoldier_t, m_rollingNoDamageDuration) == 0x209C);
#endif
// ==================================================

constexpr float c_AssultSoldier_rollingDistance = 30; // old is 24
constexpr int c_AssultSoldier_rollingMODDuration = 8;

void __fastcall EDF_HookFunction_AssultSoldier(PBYTE hmodDLL);

extern "C" {
	void __fastcall AssultSoldier_Initialization(P_NEW_AssultSoldier pIn);
}

void __fastcall AssultSoldier_SetJump220(P_NEW_AssultSoldier pIn, int state);
void __fastcall AssultSoldier_LoadAccessory290(P_NEW_AssultSoldier pIn);

void __fastcall AssultSoldier_SetAction(PBYTE pIn, P_XGS_HumanBase_ActionRDX pAction, int r8d);

void __fastcall AssultSoldier_Rolling_Set(P_NEW_AssultSoldier pIn);
void __fastcall AssultSoldier_Rolling_Active(P_NEW_AssultSoldier pIn);
void __fastcall AssultSoldier_Rolling_End(P_NEW_AssultSoldier pIn);

