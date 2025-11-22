#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "../Base/g_system.h"
#include "../Base/g_gameFunc.h"
#include "1CommonBullet.h"

extern "C" {
	CallFunc_CheckAmmoCollision fn_CheckAmmoCollision;
}

extern "C" void __fastcall newSetAmmoPosition(PXGS_Bullet pIn, float* pOut) {
	__m128 v_velocity = _mm_loadu_ps(pIn->AmmoVelocity);
	__m128 v_frameFactor = _mm_set_ps1(1.0f / 60.0f);
	float protectF3 = v_velocity.m128_f32[3];
	v_velocity = _mm_mul_ps(v_velocity, v_frameFactor);

	alignas(16) float velocity[4];
	_mm_store_ps(velocity, v_velocity);
	velocity[3] = protectF3;

	__m128 v_deltaPos = _mm_loadu_ps(pIn->AmmoDeltaPos);
	v_velocity = _mm_load_ps(velocity);
	v_deltaPos = _mm_add_ps(v_deltaPos, v_velocity);
	_mm_storeu_ps(pIn->AmmoDeltaPos, v_deltaPos);
	pIn->AmmoDeltaPos[3] = protectF3;

	__m128 v_pos = _mm_loadu_ps(pIn->AmmoCurrentPos);
	v_deltaPos = _mm_mul_ps(v_deltaPos, v_frameFactor);
	v_pos = _mm_add_ps(v_pos, v_deltaPos);
	_mm_store_ps(pOut, v_pos);
	pOut[3] = 1.0f;
}

void __fastcall AmmoClass_HookFunction_CommonBullet(PBYTE hmodDLL)
{
	// Set new bullet move coordinates function
	// edf.dll+234AA7
	// find 0F28CAC744243C0000803F0F59C80FC6C0FF0FC6C993F30F10C8
	// its top:      movups xmm0, xmmword ptr [rdi+BA0]
	UINT8 opcode_ACBSetBulletMoveCoordinates[13] = {
		0x48, 0x8B, 0x5C, 0x24, 0x78, //mov rbx, qword ptr[rsp + 78]
		0x48, 0x89, 0xF9,             //mov rcx, rdi
		0x48, 0x83, 0xC4, 0x60,       //add rsp, 60
		0x5F                          //pop rdi
	};

	int i_ACBSetBulletMoveCoordinates = 0x234AA7;
	WriteHookToProcess((void*)(hmodDLL + i_ACBSetBulletMoveCoordinates), opcode_ACBSetBulletMoveCoordinates, 13U);
	// ok, this uses JMP
	hookGameBlockWithInt3((void*)(hmodDLL + i_ACBSetBulletMoveCoordinates + 13), (uintptr_t)AmmoClass_Bullet_SetBulletMoveCoordinates);

	// get CheckAmmoCollision address
	fn_CheckAmmoCollision = (CallFunc_CheckAmmoCollision)(hmodDLL + 0x2359C0);
	// END ========================================
}

void __fastcall AmmoClass_Bullet_SetBulletMoveCoordinates(PXGS_Bullet pIn)
{
	__m128 v_velocity = _mm_loadu_ps(pIn->AmmoVelocity);
	__m128 v_frameFactor = _mm_set_ps1(1.0f / 60.0f);
	float protectF3 = v_velocity.m128_f32[3];
	v_velocity = _mm_mul_ps(v_velocity, v_frameFactor);
	
	alignas(16) float velocity[4];
	_mm_store_ps(velocity, v_velocity);
	velocity[3] = protectF3;

	__m128 v_deltaPos = _mm_loadu_ps(pIn->AmmoDeltaPos);
	v_velocity = _mm_load_ps(velocity);
	v_deltaPos = _mm_add_ps(v_deltaPos, v_velocity);
	_mm_storeu_ps(pIn->AmmoDeltaPos, v_deltaPos);
	pIn->AmmoDeltaPos[3] = protectF3;

	__m128 v_pos = _mm_loadu_ps(pIn->AmmoCurrentPos);
	v_deltaPos = _mm_mul_ps(v_deltaPos, v_frameFactor);
	v_pos = _mm_add_ps(v_pos, v_deltaPos);

	if (!pIn->HasCollision) {
	noCollision:
		_mm_storeu_ps(pIn->AmmoCurrentPos, v_pos);
		pIn->AmmoCurrentPos[3] = 1.0f;
		return;
	}

	// check is hit shield
	UINT32 bit_HitShield = 1 << Ammo::Bullet::Flag::HitShield;
	auto HitState = pIn->AmmoStateBit;
	if (HitState & bit_HitShield) {
		HitState &= ~bit_HitShield;

		pIn->AmmoStateBit = HitState;
		goto noCollision;
	}

	// with collision detection
	//edf.dll+234B4E
	alignas(16) float tempPos[4];
	_mm_store_ps(tempPos, v_pos);
	tempPos[3] = 1.0f;

	alignas(16) float hitbox[4];
	__m128 v_hitbox = _mm_set_ps1(pIn->AmmoHitboxSize);
	_mm_store_ps(hitbox, v_hitbox);
	hitbox[3] = 1.0f;

	auto pSys = XGS_GetXGSSystemPointer();

	auto fn_DetectionPathCollisionObject = Get_DetectionPathCollisionObject();
	fn_DetectionPathCollisionObject(&pIn->pad800, &pSys->ptr10, pIn->AmmoPosBB0, tempPos, hitbox);
	//
	fn_CheckAmmoCollision(pIn, pIn->AmmoPosBB0, tempPos, true);
}
