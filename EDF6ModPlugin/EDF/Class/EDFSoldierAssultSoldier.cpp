#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "Base/g_velocityCommon.h"
#include "Base/mod_inlineData.hpp"
#include "EDFSoldierCommon.h"
#include "EDFSoldierAccessoryIndex.hpp"

#include "EDFSoldierAssultSoldier.h"

extern "C" {
	void __fastcall ASMeAssultSoldierInitialization();

	uintptr_t eAssultSoldier_SetAction; // edf.dll+5518A0
	uintptr_t eAssultSoldier_SetRolling; // edf.dll+552B10
}

void __fastcall EDF_HookFunction_AssultSoldier(PBYTE hmodDLL) {
	//
	eAssultSoldier_SetAction = (uintptr_t)(hmodDLL + 0x5518A0);
	eAssultSoldier_SetRolling = (uintptr_t)(hmodDLL + 0x552B10);

	// edf.dll+54FE59
	int newSize = sizeof(NEW_AssultSoldier_t);
	WriteHookToProcessCheckECX((void*)(hmodDLL + 0x54FE59 + 1), &newSize, 4);

	// edf.dll+5501C9
	int i_rangerInit = 0x5501C9;
	hookGameBlockWithInt3((void*)(hmodDLL + i_rangerInit), (uintptr_t)ASMeAssultSoldierInitialization);
	WriteHookToProcess((void*)(hmodDLL + i_rangerInit + 15), (void*)&nop1, 1U);

	// edf.dll+17CDF28
	uintptr_t* AssultSoldier_VFT = (uintptr_t*)(hmodDLL + 0x17CDF28);

	auto newFunc220 = (uintptr_t)AssultSoldier_SetJump220;
	WriteHookToProcess(&AssultSoldier_VFT[0x220 / 8], &newFunc220, 8U);
	auto newFunc290 = (uintptr_t)AssultSoldier_LoadAccessory290;
	WriteHookToProcess(&AssultSoldier_VFT[0x290 / 8], &newFunc290, 8U);

	// =============================================================================

	// edf.dll+552B57
	BYTE setNewRollingSet[] = {
		0x48, 0xB8, 0xC7, 0x71, 0x1C, 0x6F, 0xB1, 0x2A, 0x17, 0x67,
		0xFF, 0xD0,
		0xE9, 0xFF, 0x00, 0x00, 0x00,
		0x66, 0x90
	};
	MOD::WritePTRData(&setNewRollingSet[2], AssultSoldier_Rolling_Set);
	WriteHookToProcess((void*)(hmodDLL + 0x552B57), setNewRollingSet, 19);

	// edf.dll+552D42
	BYTE setNewRollingActive[] = {
		0x48, 0xB8, 0xC7, 0x19, 0x46, 0x96, 0x02, 0x00, 0x00, 0x00,
		0xFF, 0xD0,
		0x31, 0xDB, // xor ebx, ebx
		0x48, 0x8D, 0x8F, 0x90, 0x0B, 0x00, 0x00,
		0xEB, 0x3A,
		0x0F, 0x1F, 0x44, 0x00, 0x00
	};
	MOD::WritePTRData(&setNewRollingActive[2], AssultSoldier_Rolling_Active);
	WriteHookToProcess((void*)(hmodDLL + 0x552D42), setNewRollingActive, 28);

	// edf.dll+552DD8
	BYTE setNewRollingEnd[] = {
		0x48, 0xB8, 0xC7, 0x71, 0x1C, 0x6F, 0xB1, 0x2A, 0x17, 0x67,
		0xFF, 0xD0,
		0x0F, 0x1F, 0x00
	};
	MOD::WritePTRData(&setNewRollingEnd[2], AssultSoldier_Rolling_End);
	WriteHookToProcess((void*)(hmodDLL + 0x552DD8), setNewRollingEnd, 15);
}

void __fastcall AssultSoldier_Initialization(P_NEW_AssultSoldier pIn) {
	pIn->m_bIs360Rolling = false;
	pIn->m_bIsNoDamage = false;
	pIn->m_rollingDistance = c_AssultSoldier_rollingDistance;
	pIn->m_rollingMODDuration = c_AssultSoldier_rollingMODDuration;
	pIn->m_rollingNoDamageDuration = 0;
}


void __fastcall AssultSoldier_SetJump220(P_NEW_AssultSoldier pIn, int state) {
	auto pRAW = (PBYTE)pIn;
	if (state || (pRAW[0x128] & 1) || !(MOD::ReadUInt32Data(&pRAW[0x158C]) & 0x800))
		return EDF_SoldierCommon_SetJump220(pIn, state);

	auto horizontalMove = std::fabsf(pIn->m_inputControl.x);
	if(pIn->m_bIs360Rolling){
		auto verticalMove = std::fabsf(pIn->m_inputControl.z);
		horizontalMove += verticalMove;
	}

	if(horizontalMove < 0.35f)
		return EDF_SoldierCommon_SetJump220(pIn, state);

	XGS_HumanBase_ActionRDX_t action;
	action.pFunc = eAssultSoldier_SetRolling;
	action.state = state;
	AssultSoldier_SetAction(&pRAW[0x1EF0], &action, 0);
}

void __fastcall AssultSoldier_LoadAccessory290(P_NEW_AssultSoldier pIn) {
	EDF_SoldierCommon_SetAccessoryEffects(pIn);

	auto is360Rolling = EDF_SoldierCommon_GetAccessoryValueINT32(pIn,
		AssultSoldier_Accessory_360rolling,
		0, SoldierAccessoryValueFetchType::TakeMaximum);
	if (is360Rolling > 0) pIn->m_bIs360Rolling = true;

	auto rollingDistance = EDF_SoldierCommon_GetAccessoryValueFP32(pIn,
		AssultSoldier_Accessory_rollingDistance,
		0.0f, SoldierAccessoryValueFetchType::TakeMaximum);
	if (rollingDistance > 0.0f) pIn->m_rollingDistance = c_AssultSoldier_rollingDistance * rollingDistance;

	auto rollingMODDuration = EDF_SoldierCommon_GetAccessoryValueFP32(pIn,
		AssultSoldier_Accessory_rollingMODDuration,
		0.0f, SoldierAccessoryValueFetchType::TakeMaximum);
	if (rollingMODDuration > 0.0f) pIn->m_rollingMODDuration = rollingMODDuration * c_AssultSoldier_rollingMODDuration;

	auto rollingNoDamageDuration = EDF_SoldierCommon_GetAccessoryValueFP32(pIn,
		AssultSoldier_Accessory_rollingNoDamageDuration,
		0.0f, SoldierAccessoryValueFetchType::TakeMaximum);
	if (rollingNoDamageDuration > 0.0f) pIn->m_rollingNoDamageDuration = rollingNoDamageDuration * 60.0f;
}

void __fastcall AssultSoldier_SetAction(PBYTE pIn, P_XGS_HumanBase_ActionRDX pAction, int r8d) {
	typedef void(__fastcall* func)(PBYTE pIn, P_XGS_HumanBase_ActionRDX pAction, int r8d);
	auto fn = (func)eAssultSoldier_SetAction;
	return fn(pIn, pAction, r8d);
}

void __fastcall AssultSoldier_Rolling_Set(P_NEW_AssultSoldier pIn) {
	auto angle = pIn->m_orientation1240.y;
	float sinA = std::sinf(angle);
	float cosA = std::cosf(angle);
	alignas(16) float theta[4] = { cosA, sinA, -sinA, cosA };

	alignas(16) float v_data[4];
	memcpy(v_data, &pIn->m_relativeMotionVelocity.x, 16);

	Game_GetNormalizedVelocity(v_data, pIn->m_rollingDistance);
	Velocity_GetNewOrientation(v_data, theta);

	memcpy(&pIn->m_motionVelocity1200.x, v_data, 16);
	memcpy(&pIn->m_motionVelocity1210.x, v_data, 16);

	auto new_angle = Velocity_GetNewAngle(&pIn->m_relativeMotionVelocity.x);

	auto pRAW = (PBYTE)pIn;
	MOD::WriteInt32Data(&pRAW[0x5D0], 0xE3C);
	MOD::WriteFP32Data(&pRAW[0x12D8], new_angle);
	MOD::WriteFP32Data(&pRAW[0x12DC], new_angle);

	pIn->m_mapObjectDestroyFlags = 1;

	if (pIn->m_rollingNoDamageDuration > 0) {
		GameObjectBase_SetIgnoreDamageEventMode(pIn, 1);
		pIn->m_bIsNoDamage = true;
	}
}

void __fastcall AssultSoldier_Rolling_Active(P_NEW_AssultSoldier pIn) {
	auto timer = pIn->m_stateMachine.pTimer->m_time;

	if (timer > pIn->m_rollingMODDuration) pIn->m_mapObjectDestroyFlags = 0;

	if (pIn->m_bIsNoDamage && timer > pIn->m_rollingNoDamageDuration) {
		GameObjectBase_SetIgnoreDamageEventMode(pIn, 0);
		pIn->m_bIsNoDamage = false;
	}

	// ===========================================================

	__m128 v_velocity = _mm_loadu_ps(&pIn->m_motionVelocity1200.x);
	const static __m128 v_factor = {0.975, 0.975, 0.975, 1};

	v_velocity = _mm_mul_ps(v_velocity, v_factor);
	_mm_storeu_ps(&pIn->m_motionVelocity1200.x, v_velocity);
	_mm_storeu_ps(&pIn->m_motionVelocity1210.x, v_velocity);
}

void __fastcall AssultSoldier_Rolling_End(P_NEW_AssultSoldier pIn) {
	pIn->m_mapObjectDestroyFlags = 0;

	if (pIn->m_bIsNoDamage) {
		pIn->m_bIsNoDamage = false;
		GameObjectBase_SetIgnoreDamageEventMode(pIn, 0);
	}

	EDF_SoldierCommon_ResetActionStatus(pIn, nullptr);
}
