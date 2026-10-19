#include "pch.h"
#include "utiliy.h"
#include "commonNOP.h"

#include "base_baseObject.h"

void __fastcall XGS_GameObjectBase_Initialize(PBYTE hmodDLL) {
	// edf.dll+5458F0
	BYTE clear_m_is_dead4[8] = {
		0x44, 0x89, 0xA7, 0xE8, 0x02, 0x00, 0x00, 0x90
	};
	WriteHookToProcess((void*)(hmodDLL + 0x5458F0), clear_m_is_dead4, 8U);
}

void __fastcall GameObjectBase_SetInvincible(P_XGS_GameObjectBase pIn, int isYes) {
	// edf.dll+1C33F4
	auto damage_flag = pIn->m_bitFlag380;

	if (isYes) {
		damage_flag |= 1;
	} else {
		damage_flag &= ~1;
	}
	
	pIn->m_bitFlag380 = damage_flag;
}

void __fastcall GameObjectBase_SetIgnoreDamageEventMode(P_XGS_GameObjectBase pIn, int isYes) {
	// edf.dll+1C3184
	auto damage_flag = pIn->m_bitFlag380;

	// maybe can use _bittest
	if (isYes) {
		damage_flag |= (1 << 0xB);
	} else {
		damage_flag &= ~(1 << 0xB);
	}

	pIn->m_bitFlag380 = damage_flag;
}
