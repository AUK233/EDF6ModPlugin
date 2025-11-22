#pragma once
#include "../Base/CommonStructure.hpp"

typedef struct XGS_Bullet_t {
	void* vf_table;
	char pad8[0x6C0];
	EDFVector_t v_HitInfoList;
	char pad6E0[0xA0];
	float AmmoCurrentDamage;
	char pad784[0x7C];
	char pad800[0xF0]; // Its address is required
	void* vf_8f0;
	int pad8f8, HitInPenetration;
	char pad900[0x108];
	int AmmoLifetime;
	float AmmoDamage;
	float AmmoDamageReduceFactor;
	char padA14[0x2C];
	float AmmoColor[4];
	char padA50[0xA4];
	UINT32 AmmoStateBit;
	int AmmoAliveTime; // time that has elapsed
	int AmmoActivateState; // 1 is normal, 3 is hit? It may be converted to a float as a multiple.
	char padB00[0x80];
	float AmmoCurrentPos[4], AmmoDeltaPos[4], AmmoVelocity[4], AmmoPosBB0[4];
	int RequiresCollision;
	UINT8 HasCollision;
	char padBC5[0x1B];
	float AmmoHitboxSize;
} *PXGS_Bullet;
static_assert(offsetof(XGS_Bullet_t, v_HitInfoList) == 0x6C8);
static_assert(offsetof(XGS_Bullet_t, AmmoCurrentDamage) == 0x780);
static_assert(offsetof(XGS_Bullet_t, HitInPenetration) == 0x8FC);
static_assert(offsetof(XGS_Bullet_t, AmmoLifetime) == 0xA08);
static_assert(offsetof(XGS_Bullet_t, AmmoStateBit) == 0xAF4);
static_assert(offsetof(XGS_Bullet_t, AmmoAliveTime) == 0xAF8);
static_assert(offsetof(XGS_Bullet_t, AmmoActivateState) == 0xAFC);
static_assert(offsetof(XGS_Bullet_t, AmmoCurrentPos) == 0xB80);
static_assert(offsetof(XGS_Bullet_t, AmmoPosBB0) == 0xBB0);
static_assert(offsetof(XGS_Bullet_t, RequiresCollision) == 0xBC0);
static_assert(offsetof(XGS_Bullet_t, HasCollision) == 0xBC4);
static_assert(offsetof(XGS_Bullet_t, AmmoHitboxSize) == 0xBE0);

void __fastcall AmmoClass_HookFunction_CommonBullet(PBYTE hmodDLL);

typedef void* (__fastcall* CallFunc_CheckAmmoCollision)(void* pBullet, float* pStartPos, float* pEndPos, bool CheckWithMap);
void __fastcall AmmoClass_Bullet_SetBulletMoveCoordinates(PXGS_Bullet pIn);

namespace Ammo::Bullet::Flag {
	enum AmmoState
	{
		IsDestroy = 0, // yeah, this is death flag
		IsHit = 2, // really?
		HitShield = 3,
		IsPenetration = 8,
		IsFlame = 3 + 8, // really?
	};
}
