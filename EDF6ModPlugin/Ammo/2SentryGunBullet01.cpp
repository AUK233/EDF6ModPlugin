#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "2SentryGunBullet01.h"

extern "C" {
	void __fastcall ASMammo_SentryGunBulletOpen01_FireAmmo();
	uintptr_t ammo_SentryGunBulletOpen01_FireAmmoRetAddr;
}

void __fastcall AmmoClass_HookFunction_SentryGunBullet01(PBYTE hmodDLL)
{
	// edf.dll+2A4277
	// find 4805C39E2600488987E803000048C1E82048C1E01048C1E8208987200D00004D85C00F84870000004D896150
	int i_SentryGunBulletOpen01_FireAmmo = 0x2A4277;
	hookGameBlockWithInt3((void*)(hmodDLL + i_SentryGunBulletOpen01_FireAmmo), (uintptr_t)ASMammo_SentryGunBulletOpen01_FireAmmo);
	WriteHookToProcess((void*)(hmodDLL + i_SentryGunBulletOpen01_FireAmmo + 15), (void*)&nop4, 4U);
	ammo_SentryGunBulletOpen01_FireAmmoRetAddr = (uintptr_t)(hmodDLL + i_SentryGunBulletOpen01_FireAmmo + 31);
}
