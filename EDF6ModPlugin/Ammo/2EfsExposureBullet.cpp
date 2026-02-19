#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "1CommonBullet.h"
#include "2EfsExposureBullet.h"

extern "C" {
	uintptr_t ammo_EfsExposureBulletVFT; // edf.dll+179ED48
	uintptr_t ammo_EfsExposureBullet_Func28;

	XGS_AmmoClass_FullVFT_t vft_EfsExposureBullet_AutoAlive;
}

void __fastcall AmmoClass_HookFunction_EfsExposureBullet(PBYTE hmodDLL)
{
	ammo_EfsExposureBulletVFT = (uintptr_t)(hmodDLL + 0x179ED48);
	ammo_EfsExposureBullet_Func28 = *(uintptr_t*)(ammo_EfsExposureBulletVFT + 0x28);

	/*uintptr_t i_Func28WithLaser = (uintptr_t)AmmoClass_EfsExposureBullet_Func28WithLaser;
	WriteHookToProcess((void*)(ammo_EfsExposureBulletVFT + 0x28), &i_Func28WithLaser, 8);*/

	memcpy(&vft_EfsExposureBullet_AutoAlive, (void*)(ammo_EfsExposureBulletVFT - 8), sizeof(XGS_AmmoClass_FullVFT_t));
	vft_EfsExposureBullet_AutoAlive.fn.Func28 = AmmoClass_EfsExposureBullet_Func28WithLaser;
}

void __fastcall AmmoClass_EfsExposureBullet_Func28WithLaser(char* pThis, char* pRDX)
{
	auto pBullet = (PXGS_Bullet)(pThis + 0x140);

	auto ammoLifetime = pBullet->AmmoLifetime;
	auto ammoAliveTime = pBullet->AmmoAliveTime;
	ammoAliveTime++;
	pBullet->AmmoAliveTime = ammoAliveTime;

	if (ammoAliveTime <= ammoLifetime) {
		auto pVFT = *(PXGS_AmmoClass_VFT*)pThis;
		pVFT->Func60(pThis);
	}

	original:
	auto fn28 = (CallFunc_AmmoClassFunc28)ammo_EfsExposureBullet_Func28;
	return fn28(pThis, pRDX);
}
