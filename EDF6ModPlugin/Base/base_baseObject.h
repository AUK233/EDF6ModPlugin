#pragma once
#include "SSE.hpp"
#include "base_LinkedList.h"

// edf.dll+1189C00
typedef struct XGS_SceneObject_t {
	void* vf_table;
	char pad8[0x58];
	G_Matrix3D_t m_transform;
	char padA0[0x80];
} *P_XGS_SceneObject;
#if 1
static_assert(offsetof(XGS_SceneObject_t, m_transform) == 0x60);
static_assert(sizeof(XGS_SceneObject_t) == 0x120);
#endif

// edf.dll+5456D6
typedef struct XGS_NetworkObject_t {
	void* vf_table;
	char pad8[0x40];
} *P_XGS_NetworkObject;
#if 1
static_assert(sizeof(XGS_NetworkObject_t) == 0x48);
#endif

// edf.dll+5458E2
// edf.dll+54BA70 Structure
typedef struct XGS_GameObjectBase_StatusInfo_t {
	void* vf_table;
	bool m_is_dead, unk9; char useless10[2];
	float m_durability_base, m_durability_min, m_durability_max, m_durability;
	int m_is_emergency_recharge;
	float m_energy_base, m_energy_max, m_energy;
	float m_game_object_destroy_score_adjust, m_currentTime;
	int m_team_id, m_last_team_id;
	int useless3C;
} *P_XGS_GameObjectBase_StatusInfo;
#if 1
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_is_dead) == 0x8);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_durability_base) == 0xC);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_durability_min) == 0x10);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_durability_max) == 0x14);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_durability) == 0x18);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_energy) == 0x28);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_game_object_destroy_score_adjust) == 0x2C);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_currentTime) == 0x30);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_team_id) == 0x34);
static_assert(offsetof(XGS_GameObjectBase_StatusInfo_t, m_last_team_id) == 0x38);
static_assert(sizeof(XGS_GameObjectBase_StatusInfo_t) == 0x40);
#endif

// edf.dll+545670
typedef struct XGS_GameObjectBase_t : XGS_SceneObject_t {
	void* vft_network;
	char pad128[0x48];
	G_MatrixCamera_t m_camera;
	char pad190[0x150];
	XGS_GameObjectBase_StatusInfo_t m_status;
	char pad320[0x60];
	UINT32 m_bitFlag380;
	char pad388[0x1BC];
	XGS_LinkedList_t pSquadMember;
	char pad560[0x60];
} *P_XGS_GameObjectBase;
#if 1
static_assert(offsetof(XGS_GameObjectBase_t, m_camera) == 0x170);
static_assert(offsetof(XGS_GameObjectBase_t, m_status) == 0x2E0);
static_assert(offsetof(XGS_GameObjectBase_t, m_bitFlag380) == 0x380);
static_assert(offsetof(XGS_GameObjectBase_t, pSquadMember) == 0x540);
static_assert(sizeof(XGS_GameObjectBase_t) == 0x5C0);
#endif

void __fastcall XGS_GameObjectBase_Initialize(PBYTE hmodDLL);

extern "C" {
	void __fastcall GameObjectBase_SetInvincible(P_XGS_GameObjectBase pIn, int isYes);
	void __fastcall GameObjectBase_SetIgnoreDamageEventMode(P_XGS_GameObjectBase pIn, int isYes);
}
