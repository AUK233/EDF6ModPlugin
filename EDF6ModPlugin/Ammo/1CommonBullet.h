#pragma once

typedef struct XGS_Bullet_t {
	void* vf_table;
	char pad8[0x7F8];
	char pad800[0x2F4]; // Its address is required
	UINT32 AmmoStateBit;
	char padAF8[0x88];
	float AmmoCurrentPos[4], AmmoDeltaPos[4], AmmoVelocity[4], AmmoPosBB0[4];
	int padBC0;
	UINT8 HasCollision;
	char padBC5[0x1B];
	float AmmoHitboxSize;
} *PXGS_Bullet;
static_assert(offsetof(XGS_Bullet_t, AmmoStateBit) == 0xAF4);
static_assert(offsetof(XGS_Bullet_t, AmmoCurrentPos) == 0xB80);
static_assert(offsetof(XGS_Bullet_t, AmmoPosBB0) == 0xBB0);
static_assert(offsetof(XGS_Bullet_t, HasCollision) == 0xBC4);
static_assert(offsetof(XGS_Bullet_t, AmmoHitboxSize) == 0xBE0);

void __fastcall AmmoClass_HookFunction_CommonBullet(PBYTE hmodDLL);

typedef void* (__fastcall* CallFunc_CheckAmmoCollision)(void* pBullet, float* pStartPos, float* pEndPos, bool CheckWithMap);
void __fastcall AmmoClass_Bullet_SetBulletMoveCoordinates(PXGS_Bullet pIn);

namespace Ammo::Bullet::Flag {
	enum AmmoState
	{
		HitShield = 3,
		IsFlame = 3 + 8, // really?
	};
}